# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências, para nunca se perder entre sessões. Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

---

## 2026-09-07 — Sessão 2 (FASE 4 concluída — fundação criada)

### FASE 4 — FUNDAÇÃO DO RENDERER

- [x] `Render/Core/RenderTypes.h` — handles, descritores de buffer e draw commands.
- [x] `Render/Core/RenderConfig.h` — configuração central com fallback legado e `maxBones = 200`.
- [x] `Render/Core/RenderStats.h` — contadores de draws, triângulos, binds e uploads.
- [x] `Render/RHI/IRenderDevice.h` — contrato inicial independente do backend.
- [x] `Render/State/BindState.h` — cache de shader, VAO, buffers, pipeline e 8 texture units.
- [x] `Render/Shader/ShaderManager.h` — registry/cache inicial de shaders sem alterar `CShaderGL`.
- [x] `Render/Uniforms/GlobalUBO.h` — layout do slot 0.
- [x] `Render/Uniforms/SceneUBO.h` — layout do slot 1.
- [x] `Render/Uniforms/BoneUBO.h` — layout do slot 2 com `MaxBones = 200`.
- [x] `Render/OpenGL/OpenGLRenderDevice.h/.cpp` — backend inicial isolado da legacy.

### Limites intencionais desta etapa

A FASE 4 cria a arquitetura, mas **não redireciona ainda os draws existentes**. Isso evita quebrar `CoreGLCompat`, `OGL330` e o caminho BMD já funcional. O `OpenGLRenderDevice` inicial mantém operações de buffer e draw atrás da RHI; a ligação física às APIs GL será feita durante a integração incremental das FASES 5–7.

### Próxima etapa

**FASE 5 — ImmediateRenderer:** evoluir o batching existente do `CoreGLCompat` para um componente explícito de renderização dinâmica, com ring buffer, conversão de primitivas e integração gradual dos 61 `glBegin`.

---

## 2026-09-07 — Sessão 2 (FASE 3 concluída)

### FASE 3 — MAPA DE DEPENDÊNCIAS

- [x] `RENDER_DEPENDENCY_MAP.md` criado na branch `modernization`.
- [x] Pipeline atual documentado: Winmain → CoreGLCompat / CShaderGL / OGL330.
- [x] Dependências de `CoreGLCompat`, `CShaderGL`, `New_ModelBMD`, `New_RenderBMD`, `BoneManager` e `ZzzBMD` mapeadas.
- [x] Fluxo BMD → VAO/VBO → shader → draw documentado.
- [x] Fluxo `OBJECT::BoneTransform` → bone palette → `u_Bones` documentado.
- [x] Estados `RENDER_*` e dependências de blend/depth/texture/shader/VAO identificados para futura `PipelineState`.
- [x] Censo dos 61 `glBegin` e áreas consumidoras incorporado ao mapa.
- [x] Ordem segura de migração definida, preservando `CoreGLCompat` como fallback.
- [x] Vulkan mantido fora do caminho crítico até a RHI estar estável.

**Commit FASE 3:** `51bf424e9117d3408783d4a30e0b015c7a239ddc`

### Decisão para a FASE 4

Não reescrever o renderer atual. A fundação será adicionada em paralelo.

---

## 2026-09-06 — Sessão 1 (FASE 0 + FASE 1 + FASE 2)

### FASE 0 — SEGURANÇA
- [x] Git baseline `3b3abe2` e branch `modernization`.
- [x] Build original `Global Release|x86` verificado sem erros.
- [x] Baseline GPU: NVIDIA RTX 2060 · OpenGL 4.6 Core · GLSL 4.60 · driver 610.88.
- [ ] FPS base e teste manual do cliente ainda pendentes.

### FASE 1 — REFERÊNCIAS
- [x] MuMain Yesid disponível como referência fora do repo.
- [x] Repositórios de shaders OpenGL/Vulkan levantados.
- [ ] MuMain Sven ainda não disponível no GitHub.

### FASE 2 — AUDITORIA
- [x] `LUOIS_RENDERER_AUDIT.md` criado.
- [x] CoreGLCompat, CShaderGL, OGL330/BMD e GPU skinning auditados.
- [x] 61 ocorrências reais de `glBegin` catalogadas.

## Como buildar

```bat
cd C:\MUVULKAN\Source\Main
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  Main.sln /p:"Configuration=Global Release;Platform=x86" /m /v:m
```

## Estado do git

- `main` — baseline imutável (`3b3abe2`)
- `modernization` — branch de trabalho atual
- FASE 3: `51bf424e9117d3408783d4a30e0b015c7a239ddc`
- FASE 4: concluída nesta sessão
