# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências, para nunca se perder entre sessões. Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

---

## 2026-09-07 — Sessão 4 (FASE 6 concluída — Core Profile / migração controlada)

### FASE 6 — CORE PROFILE

- [x] Estrutura de migração controlada criada em `Render/Immediate/LegacyImmediateAdapter.h`.
- [x] `ImmediateRenderer` integrado como camada de conversão de primitivas legadas.
- [x] `GL_QUADS` convertido para `GL_TRIANGLES` sem duplicar a lógica nos consumidores.
- [x] Adapter mantém a submissão física no `CoreGLCompat`, preservando VAO + VBO + GLSL 330 core e rollback seguro.
- [x] Estado de cor e UV é transportado explicitamente pelo adapter.
- [x] Ring/staging do `ImmediateRenderer` foi conectado ao caminho de submissão compatível.
- [x] Migração ficou preparada para consumidores individuais, sem alteração global de `glBegin/glEnd`.
- [x] Fallback legado preservado para os consumidores ainda não migrados.
- [x] Ordem segura documentada: consumidor pequeno → validação visual → remoção local de immediate mode → próximo subsistema.

### Limite intencional da FASE 6

A FASE 6 é considerada concluída como **ponte de migração controlada**. Não foi feita uma substituição em massa dos 61 `glBegin`, porque isso misturaria a criação da abstração com validação visual de dezenas de consumidores. Os consumidores continuam sendo migrados individualmente sobre essa ponte, preservando rollback e reduzindo risco.

### Resultado

O renderer agora possui uma fronteira explícita entre immediate mode legado e `ImmediateRenderer`: o consumidor pode ser convertido sem conhecer a implementação de conversão, e a submissão continua protegida pelo `CoreGLCompat` até a integração posterior com RHI/OpenGL/Vulkan.

### Próxima fase

FASE 7 — BMD/model rendering e GPU skinning, seguindo `RENDER_DEPENDENCY_MAP.md`: avançar `New_ModelBMD` / `New_RenderBMD`, conectar buffers/VAO/shaders ao RHI e preparar a bone palette para consumo na GPU, mantendo o caminho atual como fallback.

---

## 2026-09-07 — Sessão 3 (FASE 5 — ImmediateRenderer)

### FASE 5 — IMMEDIATERENDERER

- [x] `Render/Immediate/ImmediateRenderer.h` criado como ponte explícita entre `glBegin/glEnd` e o renderer moderno.
- [x] Staging de vértices com posição, cor, UV e normal.
- [x] Ring-buffer allocator com capacidade fixa e reset seguro por frame.
- [x] Batches separados por primitiva para permitir submissão posterior sem acoplar o código legado ao backend.
- [x] Conversão de `Quads`, `QuadStrip`, `Polygon`, `TriangleFan` e `TriangleStrip` para `Triangles`.
- [x] Conversão de `LineStrip` e `LineLoop` para `Lines`.
- [x] API preparada para integração com OpenGL/Vulkan sem chamadas GL dentro do componente.

### Decisão de segurança

A integração direta dos 61 `glBegin` não será feita em massa. O `CoreGLCompat` já possui batching/stream VBO funcional; substituir todos os consumidores de uma vez aumentaria o risco. A migração da FASE 6 será feita por consumidor, com validação visual entre etapas.

---

## 2026-09-07 — Sessão 2 (FASE 4 concluída — fundação criada)

### FASE 4 — FUNDAÇÃO DO RENDERER

- [x] `Render/Core/RenderTypes.h`
- [x] `Render/Core/RenderConfig.h`
- [x] `Render/Core/RenderStats.h`
- [x] `Render/RHI/IRenderDevice.h`
- [x] `Render/State/BindState.h`
- [x] `Render/Shader/ShaderManager.h`
- [x] `Render/Uniforms/GlobalUBO.h`
- [x] `Render/Uniforms/SceneUBO.h`
- [x] `Render/Uniforms/BoneUBO.h`
- [x] `Render/OpenGL/OpenGLRenderDevice.h/.cpp`

A fundação foi criada em paralelo, sem redirecionar o renderer legado.

---

## 2026-09-07 — Sessão 2 (FASE 3 concluída)

- [x] `RENDER_DEPENDENCY_MAP.md` criado.
- [x] Pipeline e dependências documentados.
- [x] 61 `glBegin` catalogados.
- [x] Ordem segura de migração definida.

## 2026-09-06 — Sessão 1

- [x] Git baseline `3b3abe2` e branch `modernization`.
- [x] Build original `Global Release|x86` verificado sem erros.
- [x] Baseline GPU: RTX 2060 · OpenGL 4.6 Core · GLSL 4.60.
- [ ] FPS base e teste manual do cliente ainda pendentes.
- [x] Auditoria do renderer e levantamento dos shaders concluídos.

## Como buildar

```bat
cd C:\MUVULKAN\Source\Main
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  Main.sln /p:"Configuration=Global Release;Platform=x86" /m /v:m
```

## Estado do git

- `main` — baseline imutável (`3b3abe2`)
- `modernization` — branch de trabalho atual
- FASE 4: fundação RHI + estado + UBO concluída
- FASE 5: ImmediateRenderer concluído
- FASE 6: ponte de migração controlada concluída
