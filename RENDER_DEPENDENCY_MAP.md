# RENDER_DEPENDENCY_MAP.md
# Luois Season 5.2 — FASE 3: Mapa de Dependências do Renderer

**Data:** 2026-09-07  
**Branch:** `modernization`  
**Base:** `LUOIS_RENDERER_AUDIT.md` + código atual da branch

> Objetivo da FASE 3: transformar a auditoria em um mapa operacional para a migração. O mapa identifica o caminho atual, dependências, pontos de OpenGL legado/emulado e o destino moderno de cada subsistema.

---

## 1. Visão geral do pipeline atual

```text
Winmain.cpp
   │
   ├── CoreGLCompat::Initialize()
   │       │
   │       └── emulação FFP sobre OpenGL 4.6 Core
   │
   ├── gShaderGL->Init()
   │       └── CShaderGL / shader 330 auxiliar
   │
   └── OGL330::Init()
           │
           ├── OGL330MODEL::Init()
           │       └── New_ModelBMD / shaders VBO
           │
           └── CGMShaderBMD / New_RenderBMD

Jogo / gameplay
   │
   ├── BMD / personagens / objetos ──> OGL330 ──> VAO/VBO + GLSL 330
   │                                      └──────> GPU skinning (bone palette)
   │
   └── terrain / water / effects / UI / shadows
                                  │
                                  └──> chamadas GL legadas
                                         ↓ macros
                                    CoreGLCompat
                                         ↓
                                    VBO streaming
                                         ↓
                                    OpenGL 4.6 Core
```

---

## 2. Matriz de dependências

| Subsistema | Entradas principais | Dependências atuais | GL legado/emulado? | Destino moderno | Prioridade |
|---|---|---|---|---|---|
| Winmain/contexto | janela, contexto GL | CoreGLCompat, GLEW/GL loader | Parcial | RenderDevice/RHI | Alta |
| CoreGLCompat | glBegin/glVertex, matrizes, estado | glm, OpenGL | **Sim, emulado** | ImmediateRenderer + BindState | Alta |
| CShaderGL | shaders, matrizes, MeshRenderState | CoreGLCompat, GLSL 330 | Parcial | ShaderManager + Uniform/UBO | Alta |
| MU_OpenGL/OGL330 | flags de render, textura, shader | CShaderGL, New_RenderBMD, New_ModelBMD | Parcial | RenderFrontend/RHI | Alta |
| New_ModelBMD | BMD, meshes, bones | ZzzBMD, ZzzObject, OGL330 | Não no draw principal | GPU Mesh + ModelRenderer | **Crítica** |
| New_RenderBMD | RenderMeshVAO, textura, flags | VAO, shader, CoreGLCompat | **Estado ainda via compat** | ModelRenderer + PipelineState | **Crítica** |
| BoneManager | CHARACTER/OBJECT, BoneTransform | BMD, OBJECT | Não diretamente | Bone palette / GPU skinning | Alta |
| ZzzBMD | arquivo BMD, meshes, bones | textura, modelo, animação | Não diretamente | Asset/Model layer | Alta |
| ZzzLodTerrain | height/terrain draw | CoreGLCompat, texturas | **Sim** | TerrainRenderer | Alta |
| CSWaterTerrain | água, textura, tempo | terrain + CoreGLCompat | **Sim** | WaterRenderer | Média |
| Effects | partículas, sprites, trails | CoreGLCompat, texturas | **Sim** | EffectRenderer + ImmediateRenderer | Média |
| Shadows | volumes/maps/estado | CoreGLCompat + render state | **Sim** | ShadowRenderer / render pass | Média |
| UI | primitives, texturas, fontes | CoreGLCompat | **Sim** | UIRenderer + ImmediateRenderer | Média |
| Fonts | glyphs/texturas | UI + CoreGLCompat | **Sim** | FontRenderer | Média |

---

## 3. Dependências críticas detalhadas

### 3.1 `CoreGLCompat`

**Função:** camada de compatibilidade que permite ao código antigo continuar usando a API estilo fixed-function enquanto o contexto real é OpenGL Core.

```text
Game code
  ↓
macros CoreGLCompat.h
  ↓
CoreGLCompat.cpp
  ├── state cache
  ├── matrix stacks
  ├── immediate batching
  ├── dynamic VBO
  └── GLSL 330 compat shader
  ↓
OpenGL Core
```

**Risco:** remover essa camada antes de migrar os consumidores quebra grande parte do cliente.

**Destino:** extrair seus componentes em `Render/Core`, `Render/State` e `Render/Immediate`, mantendo uma camada de compatibilidade fina durante a transição.

---

### 3.2 `CShaderGL`

**Função:** caminho shader 330 para parte do renderer e ponte entre matrizes/estado legado e GLSL.

Dependências observadas:

```text
CShaderGL
 ├── CoreGLCompat::GetFloatv()
 ├── GLSL vertex_shader.glsl
 ├── GLSL fragment_shader.glsl
 └── MeshRenderState UBO
```

**Destino:** `ShaderManager` + `RHIShader` + `ObjectUBO/SceneUBO`.

**Nota:** uniform locations já são parcialmente cacheadas em `New_ModelBMD`; a arquitetura final deve centralizar esse cache.

---

### 3.3 `New_ModelBMD` → `New_RenderBMD`

Este é o caminho prioritário para a FASE 7.

```text
BMD
 ↓
New_ModelBMD
 ├── New_Meshs[]
 ├── VAO
 ├── vertex/index buffers
 └── bone palette
       ↓
RenderMeshVAO
       ↓
New_RenderBMD
 ├── shader selection
 ├── texture binding
 ├── blend/depth state
 ├── uniforms
 ├── bone palette upload
 └── glDrawRangeElements
```

`New_RenderBMD` ainda usa funções de estado da camada de compatibilidade, apesar de o draw principal já ser VAO + shader + `glDrawRangeElements`.

**Destino:** `Models/ModelRenderer` consumindo uma interface de render independente do backend.

---

### 3.4 `BoneManager` / `OBJECT::BoneTransform`

```text
BMD animation
   ↓
OBJECT::BoneTransform[]
   ↓
New_ModelBMD
   ↓
Bone palette (até 200 bones)
   ↓
shader uniform u_Bones
   ↓
Vertex Shader
```

`BoneManager` também fornece posições de bones para gameplay/efeitos. Portanto, **não pode ser substituído por um renderer-only class**.

**Regra:** separar o cálculo/consulta de animação do transporte da palette para GPU.

---

## 4. Fluxo de estado de renderização

O código atual mistura três responsabilidades:

```text
Render intent
    ↓
flags RENDER_*
    ↓
New_RenderBMD / CoreGLCompat
    ↓
OpenGL state
```

Estados que devem ser normalizados na futura `PipelineState`:

- blend
- depth test
- depth write
- stencil
- cull
- texture binding
- shader/program
- VAO
- viewport
- color mask
- polygon mode
- alpha test/cutoff

**Regra de migração:** primeiro mapear `RENDER_*` para um estado abstrato; só depois deixar o backend OpenGL/Vulkan aplicar o estado.

---

## 5. Censo de consumidores GL legado

A auditoria encontrou **61 ocorrências reais de `glBegin` em 13 arquivos**. Isso não significa 61 caminhos diferentes: todas passam pela emulação do `CoreGLCompat`.

| Área | Arquivos de maior impacto | Ação |
|---|---|---|
| Gameplay/UI draw | `Update/InGameUpdater.cpp` | migrar para ImmediateRenderer/structured draws |
| Terrain | `ZzzLodTerrain.cpp` | TerrainRenderer |
| Effects | `ZzzEffectNoUse.cpp`, `ZzzEffectBlurSpark.cpp`, `ZzzEffectMagicSkill.cpp` | classificar vivos/mortos e migrar |
| Character/gear | `SideHair.cpp` | Model/Effect renderer |
| World/camera | `ShadowVolume.cpp`, `PhysicsManager.cpp`, `CameraMove.cpp` | avaliar se são render ou debug/visual |
| Sprite/video | `Sprite.cpp`, `VideoTextureMF.cpp` | Sprite/Video renderer |

`ZzzEffectNoUse.cpp` deve ser classificado antes de receber trabalho de migração.

---

## 6. Dependências por fase futura

```text
FASE 3  ← este documento
   │
   ├──> FASE 4: RenderTypes / RHI / BindState / ShaderManager / UBO
   │
   ├──> FASE 5: ImmediateRenderer
   │
   ├──> FASE 7: BMD + GPU Skinning
   │
   ├──> FASE 8: Terrain
   │
   └──> FASE 9+: Water / Effects / Shadows / UI / Fonts
```

Dependências bloqueadoras:

1. **FASE 4 antes de substituir o backend**, porque os subsistemas precisam de tipos/estado comuns.
2. **FASE 5 antes de migrar os 61 `glBegin`**, porque eles precisam de uma ponte estruturada.
3. **FASE 7 pode começar em paralelo com FASE 4/5**, pois BMD já possui um caminho VAO/GLSL próprio.
4. **Vulkan permanece fora do caminho crítico** até existir uma RHI estável.

---

## 7. Ordem segura de migração

```text
1. Preservar CoreGLCompat
        ↓
2. Extrair tipos/estado para Render/
        ↓
3. Centralizar shaders e uniform cache
        ↓
4. Evoluir ImmediateRenderer
        ↓
5. Consolidar BMD + GPU skinning
        ↓
6. Migrar terrain
        ↓
7. Migrar effects/water/shadows
        ↓
8. Migrar UI/fonts
        ↓
9. Reduzir CoreGLCompat a compatibilidade residual
        ↓
10. Habilitar backend Vulkan somente após RHI estabilizada
```

---

## 8. Critérios de aceite da FASE 3

- [x] Identificado o pipeline atual completo.
- [x] Identificadas dependências de `CoreGLCompat`.
- [x] Identificado caminho BMD → VAO → shader → draw.
- [x] Identificado fluxo de bone palette.
- [x] Identificados consumidores de GL legado.
- [x] Definido destino moderno por subsistema.
- [x] Definida ordem de migração sem quebrar o cliente.

**Conclusão:** a FASE 3 está documentada. O próximo passo correto é a **FASE 4 — Fundação do Renderer**, começando pelos tipos centrais, configuração, estatísticas e interfaces RHI, sem substituir ainda o pipeline funcional existente.
