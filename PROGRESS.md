# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências. Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

---

## 2026-09-07 — Sessão 5 (FASE 7 — BMD / GPU Skinning)

### FASE 7 — Infraestrutura e ponte de integração

- [x] Limite de `200` bones mantido alinhado com `MAX_BONES`/BoneUBO.
- [x] `BonePalette` em formato CPU 3x4.
- [x] Conversão explícita 3x4 → 4x4 para consumo futuro por UBO/std140.
- [x] Proteção de índice de bone no layout de quatro influências.
- [x] Normalização segura de até 4 pesos.
- [x] Layout preparado para posição, normal, UV, bone indices e bone weights.
- [x] Ponte `BMDGpuSkinningBridge` criada para ligar a representação atual de animação do BMD ao renderer moderno.
- [x] A representação atual do BMD continua lossless: cada `Vertex_t` ainda possui seu `Node` original.
- [x] Para o formato atual, o vertex é representado como uma influência única com peso `1.0`; não foram inventados pesos inexistentes no asset.
- [x] Fallback legado preservado; nenhuma ativação global do novo skinning foi feita.

### Integração existente auditada

O caminho `New_ModelBMD` / `New_RenderBMD` já possui VAO/VBO, IBO, palette de bones e envio de palette por uniform. A auditoria confirmou que o asset BMD atual fornece **um bone por vértice** (`Vertex_t::Node`), portanto não é correto fabricar quatro influências. A evolução para quatro influências reais depende de dados de pesos no formato de asset ou de uma etapa posterior de conversão.

### Pendências da FASE 7

- [ ] Adicionar os novos arquivos ao `Main.vcxproj` quando a integração física for ativada.
- [ ] Alterar o VAO real para transportar `uvec4` + `vec4` de influências.
- [ ] Substituir o palette uniforme atual por BoneUBO/std140 no caminho moderno.
- [ ] Conectar diretamente `BoneTransform` → `BonePalette` no ponto de renderização, sem duplicar a animação CPU.
- [ ] Skin de normais no vertex shader.
- [ ] Validação visual de animação BMD.
- [ ] Teste de build `Global Release|x86` no ambiente local.
- [ ] Promover o caminho GPU somente após validação visual e build limpo.

### Decisão de segurança

FASE 7 continua isolada. O renderer legado não foi alterado e o caminho `jdk_shader_local330` não foi ativado globalmente. O próximo passo é integrar o VAO/UBO real do caminho moderno, mantendo fallback por falha de dados, shader ou inicialização.

---

## 2026-09-07 — Sessão 4 (FASE 6 — Core Profile / migração controlada)

- [x] Estrutura de migração controlada criada em `Render/Immediate/LegacyImmediateAdapter.h`.
- [x] `ImmediateRenderer` integrado como camada de conversão de primitivas legadas.
- [x] `GL_QUADS` convertido para `GL_TRIANGLES`.
- [x] Adapter mantém submissão física no `CoreGLCompat`, preservando rollback seguro.
- [x] Migração individual preparada; substituição em massa dos 61 `glBegin` não feita.

## 2026-09-07 — Sessão 3 (FASE 5 — ImmediateRenderer)

- [x] `Render/Immediate/ImmediateRenderer.h` criado.
- [x] Staging de vértices com posição, cor, UV e normal.
- [x] Conversão de primitivas legadas para batches modernos.
- [x] API preparada para integração OpenGL/Vulkan.

## 2026-09-07 — Sessão 2 (FASE 4 — Fundação)

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

## 2026-09-07 — Sessão 2 (FASE 3)

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
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild.exe" Main.sln /p:"Configuration=Global Release;Platform=x86" /m /v:m
```

## Estado do git

- `main` — baseline imutável (`3b3abe2`)
- `modernization` — branch de trabalho atual
- FASE 4: concluída
- FASE 5: concluída
- FASE 6: ponte concluída
- FASE 7: infraestrutura + ponte BMD concluídas; integração física ainda pendente
