# LUOIS_RENDERER_AUDIT.md
**Fase 2 — Auditoria do Renderer da Luois 5.2**
**Versão:** 1.0 (completa — CoreGLCompat, OGL330/BMD, BoneManager, ZzzBMD, terrain)
**Data:** 2026-09-06
**Branch de trabalho:** `modernization`

> Arquivos ISO-8859-1 (ex.: `CShaderGL.cpp`) precisam de `iconv -f ISO-8859-1 -t UTF-8` antes de leitura com ferramentas modernas.

---

## 1. DESCOBERTA PRINCIPAL — O cliente já roda OpenGL 4.6 Core com FFP emulada

O cenário real é mais avançado do que o plano assumia:

1. `Winmain.cpp` cria contexto **OpenGL 4.6 Core** e registra `Cliente/opengl.log`
   (máquina atual: NVIDIA RTX 2060, driver 610.88, GLSL 4.60 — ver PROGRESS.md).
2. `CoreGLCompat::Initialize()` é chamada **incondicionalmente** em `Winmain.cpp:1440`;
   em caso de falha exibe "Falha ao inicializar o backend OpenGL Core".
3. `CoreGLCompat.h` remapeia **~57 funções da FFP via macros** (`glBegin`, `glVertex*`,
   `glColor*`, `glMatrixMode`, `glEnable/glDisable`, `glBindTexture`, matrizes, fog,
   alpha test, `glPushAttrib`…). Todo o resto do código do jogo chama a "GL legada"
   sem saber — na prática chama a emulação.
4. `CoreGLCompat.cpp` começa com `#define CORE_GL_COMPAT_IMPLEMENTATION` + `#undef`
   de todas as macros: é o **único TU do projeto que fala com a OpenGL real**.

**Consequência:** a FASE 6 do plano (Core Profile) já está cumprida no nível de
*emulação*; não há mais fixed function real. O trabalho restante é de
*arquitetura e performance* (FASE 4, 5, 7+), não de boot.

---

## 2. CoreGLCompat.cpp (1.100 linhas) — o ImmediateRenderer já existe

É, na prática, o "ImmediateRenderer + BindState embrionário" das FASES 4–5 do plano:

**Pipeline emulado**
- Programa GLSL `330 core` embutido no .cpp (vertex + fragment) com uniforms:
  `uProjection`, `uModelView`, `uTexture`, `uTextureEnabled`, `uAlphaTestEnabled`,
  `uAlphaReference`, `uTextureEnvironment` (MODULATE/REPLACE/ADD), e fog completa
  (`uFogEnabled/uFogColor/uFogStart/uFogEnd` — fog linear).
- `CoreVertex { float pos[3]; float color[4]; float tex[2]; }` = 36 bytes/vértice.

**Fluxo immediate mode**
- `Begin/End`: acumula vértices em `std::vector`; `End()` converte primitivas
  (GL_QUADS → 2 triângulos, QUAD_STRIP, POLYGON, TRIANGLE_FAN/STRIP, LINE_STRIP/LOOP).
- **Batching:** vértices ficam em `g_pendingVertices` (até 65.536) e só sobem na
  mudança de primitiva ou lotação (`FlushPending`) — batching real, não draw por Begin/End.
- **Streaming VBO de 8 MB** com orphaning (`glBufferData(NULL)` no wrap) +
  `glBufferSubData`; **um único VAO global**.

**Estado**
- Matrizes glm com stacks MODELVIEW/PROJECTION (Push/Pop/Translate/Rotate/Scale/
  Ortho/Perspective); `glGetFloatv` emulado devolve `GL_MODELVIEW_MATRIX`,
  `GL_PROJECTION_MATRIX` e `GL_CURRENT_COLOR` do estado emulado.
- `PipelineState` cacheia blend/depth/stencil/colorMask/lineWidth/pointSize/
  viewport/polygonMode; `g_capabilityState` cacheia `glEnable/glDisable` —
  **embrião do BindState (seção 10 do plano)**.
- `PushAttrib/PopAttrib` salva/restaura estado emulado **e** consulta GL real
  (`glIsEnabled`, `glGetIntegerv`) para blend/depth/cull/stencil/texture binding.
- `ApplyFogUniforms(program)` aplica fog a programas externos via cache por
  programa + `g_fogVersion` — a ponte para os shaders do CShaderGL.
- `EnableClientState/VertexPointer/DrawArrays`: emula vertex array client-side
  (apenas GL_VERTEX_ARRAY float) copiando para `CoreVertex` no CPU.
- `Normal3f` é **no-op** e `GL_LIGHTING`/`GL_ALPHA_TEST`/`GL_FOG`/`GL_TEXTURE_2D`
  são tratados como "legacy capability" (não vão ao GL real) — não há lighting
  na emulação; a iluminação vem de vertex colors pré-calculados.

**Pontos fracos (o custo da emulação)**
- 100% CPU: todo vértice do jogo passa por `std::vector` + memcpy.
- Um único programa global → sem lighting, sem variações por material.
- `FlushPending()` dispara a cada mudança de matriz/estado → muitos uploads
  pequenos (o MU alterna estado com muita frequência).
- `PushAttrib` faz chamadas GL de leitura (potenciais syncs).

---

## 3. CShaderGL (260 linhas) — o pipeline BMD moderno existe, mas está DORMENTE

- Singleton (`gShaderGL`), carrega `Data\Effect\Shader\vertex_shader.glsl` +
  `fragment_shader.glsl` (confirmado que existem em `Cliente/Data/Effect/Shader/`).
- **UBO `uboMeshState`** = `MeshRenderState` (RenderFlag, Alpha, BlendMeshTexCoordU/V,
  BlendMeshLight — alinhado 16B) no binding 0 → protótipo do ObjectUBO do plano.
- `BuildModelMatrix` e `BuildModelMatrixFrom34` (matriz 3x4 de bone → mat4 glm)
  — é a semente do caminho BoneUBO/GPU Skinning (FASE 7).
- `Projection()` lê GL_MODELVIEW/GL_PROJECTION **do estado emulado** do CoreGLCompat
  (funciona porque `GetFloatv` é interceptado).
- `readshader()` lê texto plano (nome enganoso — não há compressão).
- Erros de **link silenciados** (logs comentados) — risco de falha invisível.
- Uniform setters (`setInt/setFloat/…`) fazem `glGetUniformLocation` **por chamada**
  (sem cache) — refatorar no ShaderManager da FASE 4.

---

## 4. MU_OpenGL (114 linhas) — gerenciador texture→shader do caminho 330

- Namespace `OGL330`: `TextureManager` mapeia `RENDER_*` → `BITMAP_*` → `SHADER_330_*`
  (`AddTexturShader`), `m_ShaderState`, `m_FlagRenderContainer`.
- `OGL330::Init()` cria `g_NewRenderBMD = new CGMShaderBMD()` (New_RenderBMD) e
  chama `OGL330MODEL::Init()` (New_ModelBMD).
- Tudo dentro de `#if jdk_shader_local330` (ver §5) → **nunca roda hoje**.

---

## 5. FLAG `jdk_shader_local330` — ATIVO (correção da v0.1)

> **Correção:** a v0.1 concluiu erroneamente que o flag não existia. Ele está
> definido e o pipeline OGL330 **está compilado e inicializado em runtime**.

- `#define jdk_shader_local330 1` em `Defined_Global.h:41`, incluído via
  `stdafx.h:88` — vale para **todas** as configs do Main.
- Boot sequence (`Winmain.cpp`):
  1. `CoreGLCompat::Initialize()` — emulação FFP (linha ~1440)
  2. `gShaderGL->Init()` — shader 2D órfão (linha 3446)
  3. `OGL330::Init()` — pipeline BMD/VBO (linha 3633)
- `OGL330::Init()` seta `m_ShaderState = true`; `SetShaderState()` existe mas
  **nunca é chamado** — o caminho GPU fica sempre ligado após boot.
- **Conclusão:** o jogo opera em **dois pipelines paralelos**:
  - **BMD (personagens, objetos, itens):** VAO + GLSL 330 + GPU skinning
  - **Resto (terrain, efeitos, UI, sombras legadas):** CoreGLCompat emulado

---

## 6. Censo `glBegin` (fonte, sem `glew.h`)

| Arquivo | glBegin |
|---|---|
| Update/InGameUpdater.cpp | 25 |
| ZzzLodTerrain.cpp | 9 |
| ZzzEffectNoUse.cpp | 8 |
| ZzzEffectBlurSpark.cpp | 4 |
| SideHair.cpp | 4 |
| ZzzEffectMagicSkill.cpp | 2 |
| Sprite.cpp | 2 |
| ShadowVolume.cpp | 2 |
| PhysicsManager.cpp | 2 |
| CameraMove.cpp | 2 |
| VideoTextureMF.cpp | 1 |
| **Total real** | **61** |

Obs.: `ZzzEffectNoUse.cpp` sugere código morto — confirmar antes de migrar.
Obs.: todos esses `glBegin` já caem na emulação (macros) — migrá-los é
reorganização, não sobrevivência.

---

## 7. Implicações para o plano (ajustes recomendados)

1. **FASE 6 (Core Profile)** → reclassificar como *concluída na emulação*;
   reescopo para "converter os 61 glBegin restantes em draws estruturados".
2. **FASE 5 (ImmediateRenderer)** → não criar do zero: **evoluir o CoreGLCompat**
   (ring buffer real, flush menos agressivo, VAO por formato de vértice).
3. **FASE 4 (BindState)** → o `PipelineState` do CoreGLCompat é o ponto de partida;
   extrair para `BindState.h` próprio.
4. **FASE 7 (BMD/GPU Skinning)** → ativar/absorver o caminho dormente
   (`jdk_shader_local330`: CShaderGL + New_ModelBMD/New_RenderBMD) antes de
   escrever código novo; provavelmente já contém metade do trabalho.
5. **Risco de regressão visual:** a emulação não tem lighting (Normal3f no-op);
   qualquer shader novo que assuma normais precisará integrar os vertex colors
   do legado para manter a aparência.
6. **Shaders de referência (`opengl-main`)** assumem atributos/pipelines próprios —
   mapear a correspondência com `MeshRenderState`/`CoreVertex` durante a FASE 7.

---

## 8. Próximos alvos de auditoria (ordem proposta)

1. `New_ModelBMD.cpp/.h` (730 linhas) + `New_RenderBMD.cpp/.h` (133) — o pipeline dormente
2. `BoneManager.cpp/.h` (157) — skinning CPU atual
3. `ZzzBMD.cpp/.h` (3.650) — formato/loader BMD
4. `ZzzLodTerrain` / `CSWaterTerrain` (terrain e water)
5. `ZzzObject` / `ZzzEffect*` / Interface — maior volume de draws emulados
