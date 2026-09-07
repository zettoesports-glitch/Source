# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências, para nunca se perder entre sessões. Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

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

A integração direta dos 61 `glBegin` não será feita em massa nesta etapa. O `CoreGLCompat` já possui batching/stream VBO funcional; substituir todos os consumidores de uma vez aumentaria o risco. O próximo passo é conectar **um primeiro consumidor controlado** ao `ImmediateRenderer`, validar visualmente e então repetir por subsistema.

### Próxima etapa

**FASE 6 — CORE PROFILE:** começar a migração controlada de `glBegin/glEnd`, começando pelos consumidores de menor risco e mantendo `CoreGLCompat` como fallback.

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
- FASE 5: staging + ring buffer + conversão de primitivas concluídos
