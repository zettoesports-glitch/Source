# MODERNIZATION_PLAN_V2.md
# Luois Season 5.2 — Plano Definitivo de Modernização

**Status:** Plano mestre V2  
**Objetivo:** Modernizar progressivamente a Luois Season 5.2, preservando compatibilidade e preparando o cliente para OpenGL moderno e futuro Vulkan.

---

# 1. VISÃO GERAL

Este projeto não consiste em trocar a source inteira.

A estratégia é preservar:

- Gameplay original
- Protocol
- GameServer
- DataServer
- JoinServer
- ConnectServer
- formatos BMD
- mapas
- animações
- assets existentes
- lógica de personagens
- sistemas Season 5.2

A modernização acontecerá principalmente no cliente gráfico.

```text
LUOIS SEASON 5.2
      │
      ▼
MODERNIZAÇÃO GRADUAL
      │
 ┌────┼─────┐
 ▼    ▼     ▼
OpenGL RHI Vulkan
```

---

# 2. PRINCÍPIO FUNDAMENTAL

## Não reescrever tudo

```text
LEGACY FUNCIONANDO
       +
NOVA CAMADA PARALELA
       +
MIGRAÇÃO GRADUAL
       =
MENOR RISCO
```

Cada etapa deve:

1. Compilar
2. Abrir o cliente
3. Entrar no jogo
4. Renderizar corretamente
5. Manter comportamento anterior
6. Permitir rollback

---

# 3. OS CINCO PILARES

## 3.1 Luois 5.2 / 12345678 — Base Principal

Responsável por:

- Cliente
- Main
- GameServer
- DataServer
- JoinServer
- ConnectServer
- Gameplay
- BMD
- Animações
- Terrain
- Effects
- Rede

**A Luois é o corpo do projeto. Não será abandonada.**

---

## 3.2 MuMain Sven — Referência de Migração

Usar para estudar:

- evolução da Luois
- GPU Skinning
- retirada gradual do Fixed Function Pipeline
- UBO
- ImmediateRenderer
- RHI
- arquitetura moderna

```text
Luois Legacy
     ↓
OpenGL Moderno
     ↓
GPU Skinning
     ↓
RHI
```

---

## 3.3 MuMain Workspace / Yesid — Referência Arquitetural

Usar para estudar:

- arquitetura moderna
- organização do workspace
- separação de componentes
- builds x86/x64
- documentação
- estratégias de migração

Não copiar cegamente. Usar como referência de engenharia.

---

## 3.4 Repositório OpenGL — Implementação dos Shaders

Usar para:

- Models
- Terrain
- Water
- Effects
- Shadows
- Cloth
- Interface
- Fonts
- GPU Skinning
- Pipeline GLSL

```text
Luois BMD
    ↓
Adaptador C++
    ↓
OpenGL Renderer
    ↓
Shaders GLSL
    ↓
GPU
```

---

## 3.5 Repositório Vulkan — Arquitetura Futura

Usar para estudar:

- Separação CPU/GPU
- Descriptors
- Pipelines
- Buffers
- SPIR-V
- Arquitetura backend

**Não iniciar Vulkan agora.**

Pensar a arquitetura para permitir Vulkan futuramente.

---

# 4. MAPA DE RESPONSABILIDADES

| Fonte | Função |
|---|---|
| Luois 5.2 | Base real |
| MuMain Sven | Caminho técnico |
| MuMain Yesid | Arquitetura moderna |
| OpenGL | Shaders e pipeline |
| Vulkan | Backend futuro |

---

# 5. ARQUITETURA FINAL

```text
Game Engine / MU Logic
        │
        ▼
Render Frontend
        │
        ├── Models
        ├── Terrain
        ├── Water
        ├── Effects
        ├── Cloth
        ├── Shadows
        ├── Interface
        └── Fonts
        │
        ▼
Renderer API / RHI
        │
        ├───────────────┐
        ▼               ▼
 OpenGL Backend    Vulkan Backend
```

---

# 6. RHI — RENDER HARDWARE INTERFACE

O jogo não deve depender diretamente de OpenGL.

```text
IRenderDevice
│
├── CreateBuffer()
├── DestroyBuffer()
├── CreateTexture()
├── DestroyTexture()
├── CreateShader()
├── CreatePipeline()
├── BindPipeline()
├── BindBuffer()
├── BindTexture()
├── Draw()
└── DrawIndexed()
```

Implementações:

```text
OpenGLRenderDevice
VulkanRenderDevice
```

---

# 7. ESTRUTURA DE PASTAS PROPOSTA

```text
Source/Main/source/Render/
│
├── Core/
│   ├── RenderTypes.h
│   ├── RenderConfig.h
│   ├── RenderStats.h
│   └── Renderer.h
│
├── RHI/
│   ├── IRenderDevice.h
│   ├── RHIBuffer.h
│   ├── RHITexture.h
│   ├── RHIShader.h
│   └── RHIPipeline.h
│
├── State/
│   ├── BindState.h
│   └── PipelineState.h
│
├── Uniforms/
│   ├── GlobalUBO.h
│   ├── SceneUBO.h
│   └── BoneUBO.h
│
├── Shader/
├── Immediate/
├── Models/
├── Terrain/
├── Effects/
├── Shadows/
├── Cloth/
├── UI/
├── OpenGL/
└── Vulkan/
```

---

# 8. GLOBAL UNIFORM ARCHITECTURE

## Slot 0 — GlobalUBO

```text
View Matrix
Projection Matrix
ViewProjection Matrix
Camera Position
World Time
Screen Size
```

## Slot 1 — SceneUBO

```text
Fog
Sun Direction
Light Position
Ambient Light
Environment
Wind
Water
```

## Slot 2 — BoneUBO

```text
BoneMatrices[MAX_BONES]
```

## Slot 3 — ObjectUBO

```text
Model Matrix
Normal Matrix
Object Parameters
```

---

# 9. REGRA CRÍTICA — MAX_BONES

```cpp
constexpr uint32_t MAX_BONES = 200;
```

Nunca assumir que o número de bones de um mesh isolado é suficiente.

```text
PERSONAGEM COMPLETO
        ↓
Bone Palette Completa
        ↓
MAX_BONES = 200
        ↓
GPU Skinning
```

---

# 10. BINDSTATE / CACHE DE ESTADO

```text
BindState
├── CurrentProgram
├── CurrentVAO
├── CurrentArrayBuffer
├── CurrentIndexBuffer
├── CurrentTextureUnits
├── CurrentFramebuffer
└── CurrentPipeline
```

Objetivo:

```text
Estado solicitado
      ↓
Já está ativo?
  ┌───┴───┐
 Sim      Não
 │         │
Skip      Bind
```

---

# 11. IMMEDIATERENDERER

O MU antigo possui muito código:

```cpp
glBegin();
glVertex();
glTexCoord();
glColor();
glEnd();
```

O ImmediateRenderer será a ponte:

```text
LEGACY DRAW CALL
       ↓
ImmediateRenderer
       ↓
CPU Temporary Buffer
       ↓
Dynamic VBO / Ring Buffer
       ↓
GPU
```

Será usado para:

- UI
- partículas
- efeitos
- primitives
- cloth
- elementos antigos

---

# 12. CORE PROFILE E RETIRADA DO FIXED FUNCTION PIPELINE

```text
LEGACY
glMatrixMode
glLoadIdentity
glBegin
glEnd
glVertex
glTexCoord
       ↓
MIGRAÇÃO
       ↓
CORE PROFILE
VAO
VBO
IBO
Shaders
UBO
```

Migrar subsistema por subsistema.

---

# 13. SHADER SYSTEM

```text
ShaderManager
│
├── Load()
├── Compile()
├── Link()
├── Cache()
├── Reload()
├── Bind()
└── Destroy()
```

Organização:

```text
Shaders/
├── Models/
├── Terrain/
├── Water/
├── Effects/
├── Shadows/
├── Cloth/
├── UI/
└── Fonts/
```

---

# 14. MIGRAÇÃO DO CShaderGL

O sistema atual possui:

- GLSL
- Uniforms
- UBO
- Matrizes
- Gerenciamento de programas

Estratégia:

```text
CShaderGL
   ↓
Auditoria
   ↓
Separar responsabilidades
   ├── ShaderProgram
   ├── ShaderManager
   └── Uniform System
```

Não apagar imediatamente.

---

# 15. MODELOS BMD

Arquivos prioritários:

```text
New_ModelBMD
New_RenderBMD
ZzzBMD
BoneManager
```

Pipeline:

```text
BMD File
   ↓
CPU Model Data
   ↓
Mesh Conversion
   ↓
Vertex Buffer
Index Buffer
Bone Weights
Bone Indices
   ↓
GPU
```

---

# 16. GPU SKINNING

Pipeline antigo:

```text
Bones
 ↓
CPU transforma vértices
 ↓
CPU envia geometria
 ↓
GPU desenha
```

Pipeline moderno:

```text
Bones
 ↓
Bone Matrices
 ↓
BoneUBO / Texture Buffer
 ↓
GPU Vertex Shader
 ↓
Skinning
```

---

# 17. ETAPAS DO GPU SKINNING

1. Preservar animação CPU
2. Extrair matriz final de cada bone
3. Enviar matriz para GPU
4. Adicionar BoneIndices
5. Adicionar BoneWeights
6. Implementar shader
7. Comparar CPU vs GPU
8. Migrar equipamento
9. Migrar personagens
10. Remover skinning CPU somente após validação

---

# 18. TERRAIN

Referências:

```text
ZzzLodTerrain
CSWaterTerrain
OpenGL terrain shaders
Vulkan terrain shaders
```

```text
Terrain Data
     ↓
Heightmap
     ↓
GPU Terrain Mesh
     ↓
Terrain Shader
     ↓
Lighting / Fog / Shadows
```

---

# 19. WATER

```text
Water Parameters
├── Time
├── Movement
├── Wind
├── Reflection
└── Distortion
```

Migrar separado do terrain.

---

# 20. EFFECTS

Categorias:

- Partículas
- Sprites
- Glow
- Fire
- Smoke
- Magic
- Trail
- Blending

```text
Effect System
      ↓
Effect Renderer
      ↓
Dynamic Buffers
      ↓
Shaders
```

---

# 21. SHADOWS

```text
Light
 ↓
Shadow Camera
 ↓
Shadow Map
 ↓
Depth Texture
 ↓
Scene Rendering
 ↓
Shadow Sampling
```

Etapas:

1. Manter sistema atual
2. Entender shaders
3. Criar ShadowRenderer
4. Criar Render Pass
5. Integrar modelos
6. Integrar terrain

---

# 22. CLOTH

```text
Cloth Simulation
      ↓
Cloth Mesh
      ↓
Dynamic GPU Buffer
      ↓
Cloth Shader
```

Não misturar com ModelRenderer.

---

# 23. UI E INTERFACE

```text
Legacy UI
    ↓
ImmediateRenderer
    ↓
Dynamic Buffer
    ↓
UI Shader
```

Migração gradual.

---

# 24. FONTS

```text
FontManager
     ↓
Glyph Atlas
     ↓
Font Renderer
     ↓
Text Shader
```

---

# 25. PROFILING

```text
FrameProfiler
├── CPU Frame Time
├── GPU Frame Time
├── Draw Calls
├── Triangles
├── Shader Switches
├── Texture Switches
├── Buffer Uploads
└── Memory
```

Regra:

```text
Medir
  ↓
Identificar gargalo
  ↓
Otimizar
  ↓
Medir novamente
```

---

# 26. ROADMAP DEFINITIVO

## FASE 0 — SEGURANÇA

- [x] Backup completo *(git init + commit baseline `3b3abe2`, 2026-09-06 — ver PROGRESS.md)*
- [x] Branch de modernização *(branch `modernization`)*
- [x] Build original funcionando *(Global Release|x86 compila sem erros — VS 18, toolset v143)*
- [ ] Cliente original testado
- [ ] FPS base registrado *(GPU/driver já registrados: RTX 2060, GL 4.6 Core, driver 610.88)*

## FASE 1 — INVESTIGAÇÃO DOS REPOSITÓRIOS

- [ ] Luois 5.2
- [ ] MuMain Sven
- [ ] MuMain Yesid
- [ ] OpenGL shaders
- [ ] Vulkan shaders

Resultado:

```text
ARCHITECTURE_COMPARISON.md
```

## FASE 2 — AUDITORIA DA LUOIS

Analisar arquivo por arquivo:

- [ ] MU_OpenGL
- [ ] CShaderGL
- [ ] CoreGLCompat
- [ ] New_ModelBMD
- [ ] New_RenderBMD
- [ ] ZzzBMD
- [ ] BoneManager
- [ ] Terrain
- [ ] Water
- [ ] Effects
- [ ] Shadows
- [ ] Interface

Resultado:

```text
LUOIS_RENDERER_AUDIT.md
```

## FASE 3 — MAPA DE DEPENDÊNCIAS

```text
Arquivo
  ↓
Função
  ↓
Quem chama
  ↓
Dependências
  ↓
OpenGL Legacy?
  ↓
Destino Moderno
```

Resultado:

```text
RENDER_DEPENDENCY_MAP.md
```

## FASE 4 — FUNDAÇÃO DO RENDERER

- [ ] RenderTypes
- [ ] RenderConfig
- [ ] RenderStats
- [ ] IRenderDevice
- [ ] OpenGLRenderDevice inicial
- [ ] BindState
- [ ] ShaderManager
- [ ] GlobalUBO
- [ ] SceneUBO
- [ ] BoneUBO

## FASE 5 — IMMEDIATERENDERER

- [ ] Dynamic VBO
- [ ] Ring Buffer
- [ ] Primitive conversion
- [ ] Integração gradual com código antigo

## FASE 6 — CORE PROFILE

- [ ] Identificar glBegin/glEnd
- [ ] Substituir progressivamente
- [ ] VAO
- [ ] VBO
- [ ] IBO
- [ ] Remover Fixed Function somente quando seguro

## FASE 7 — BMD E GPU SKINNING

- [ ] Mapear BMD
- [ ] Mapear BoneManager
- [ ] Criar GPU mesh
- [ ] BoneIndices
- [ ] BoneWeights
- [ ] BoneUBO
- [ ] Model shader
- [ ] Comparação CPU/GPU
- [ ] Migração gradual

## FASE 8 — TERRAIN

- [ ] Mesh moderno
- [ ] Terrain shaders
- [ ] Fog
- [ ] Lighting
- [ ] Shadows

## FASE 9 — WATER

- [ ] Water parameters
- [ ] Water shader
- [ ] Movimento
- [ ] Wind
- [ ] Distortion

## FASE 10 — EFFECTS

- [ ] EffectRenderer
- [ ] Dynamic buffers
- [ ] Particles
- [ ] Trails
- [ ] Blending

## FASE 11 — SHADOWS

- [ ] ShadowRenderer
- [ ] Shadow Map
- [ ] Depth pass
- [ ] Model shadows
- [ ] Terrain shadows

## FASE 12 — CLOTH

- [ ] Dynamic mesh
- [ ] Cloth buffers
- [ ] Cloth shaders

## FASE 13 — UI E FONTS

- [ ] ImmediateRenderer
- [ ] UI shader
- [ ] Font atlas
- [ ] Text renderer

## FASE 14 — PROFILING E OTIMIZAÇÃO

- [ ] FrameProfiler
- [ ] CPU profiling
- [ ] GPU profiling
- [ ] Draw call analysis
- [ ] State switch analysis

## FASE 15 — OPENGL MODERNO COMPLETO

Meta:

```text
OpenGL moderno
GPU Skinning
UBO
VBO
VAO
State Cache
Shaders organizados
Profiler
```

## FASE 16 — PREPARAÇÃO VULKAN

Somente depois do OpenGL estar estável:

- [ ] Validar RHI
- [ ] Separar backend
- [ ] Criar VulkanRenderDevice
- [ ] SPIR-V
- [ ] Descriptors
- [ ] Pipelines
- [ ] Command Buffers

---

# 27. REGRA DE OURO

Nunca:

```text
Alterar 50 arquivos
       ↓
Compilar
       ↓
Descobrir que quebrou
       ↓
Não saber onde
```

Sempre:

```text
1 mudança
    ↓
Compilar
    ↓
Testar
    ↓
Commit
    ↓
Próxima mudança
```

---

# 28. ESTRATÉGIA DE GIT

```text
main
│
└── modernization
    │
    ├── renderer-foundation
    ├── immediate-renderer
    ├── gpu-skinning
    ├── terrain
    ├── effects
    ├── shadows
    └── vulkan-preparation
```

---

# 29. CHECKPOINTS

## Checkpoint A
Cliente original funciona após adicionar infraestrutura.

## Checkpoint B
ShaderManager funcionando sem regressão.

## Checkpoint C
ImmediateRenderer funcionando.

## Checkpoint D
Primeiro BMD renderizado pelo pipeline moderno.

## Checkpoint E
GPU Skinning visualmente idêntico.

## Checkpoint F
Terrain moderno.

## Checkpoint G
Effects modernos.

## Checkpoint H
OpenGL moderno estável.

## Checkpoint I
RHI pronta para Vulkan.

---

# 30. O QUE NÃO FAZER

- Não copiar repositórios inteiros
- Não misturar OpenGL e Vulkan diretamente
- Não apagar CoreGLCompat cedo
- Não substituir todo renderer de uma vez
- Não mudar BMD sem entender o formato
- Não remover CPU Skinning antes de validar GPU Skinning
- Não otimizar sem profiling
- Não iniciar Vulkan antes do OpenGL moderno estar estável

---

# 31. RESULTADO FINAL ESPERADO

```text
LUOIS 5.2 MODERNIZADA

Game Logic
    │
    ▼
Modern Render Frontend
    │
    ├── Models
    ├── GPU Skinning
    ├── Terrain
    ├── Water
    ├── Effects
    ├── Shadows
    ├── Cloth
    ├── UI
    └── Fonts
    │
    ▼
RHI
    │
    ├── OpenGL Moderno
    │
    └── Vulkan Futuro
```

---

# 32. DEFINIÇÃO FINAL DO PROJETO

A estratégia definitiva é:

> Usar a Luois 5.2 como base real, estudar profundamente o caminho técnico do MuMain Sven, aproveitar a organização e evolução arquitetural do MuMain/Yesid, implementar os shaders do repositório OpenGL e preparar a arquitetura para receber Vulkan no futuro.

Não é uma simples atualização gráfica.

É uma modernização progressiva da engine gráfica mantendo a essência e compatibilidade da Luois Season 5.2.

---

# 33. PRÓXIMA ETAPA IMEDIATA

Antes de escrever código:

1. Auditoria completa da Luois arquivo por arquivo
2. Auditoria do MuMain
3. Auditoria dos shaders OpenGL
4. Auditoria dos shaders Vulkan
5. Matriz comparativa
6. Mapa de dependências
7. Somente então iniciar a FASE 4

---

**FIM DO PLANO MESTRE V2**
