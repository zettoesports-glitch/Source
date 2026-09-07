# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências. Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

---

## 2026-09-07 — Sessão 6 (FASE 7 — auditoria e correções seguras)

### Auditoria FASE 1 → FASE 7

- [x] FASE 1: baseline Git/build/GPU documentado; FPS base e teste manual continuam pendentes.
- [x] FASE 2: auditoria do renderer e caminho OpenGL moderno/compatibilidade confirmados.
- [x] FASE 3: mapa de dependências e 61 `glBegin` catalogados.
- [x] FASE 4: fundação RHI/Render criada; integração física ainda não redireciona o legado.
- [x] FASE 5: ImmediateRenderer criado; conversão de primitivas preparada, mas submissão ainda passa pelo caminho compatível.
- [x] FASE 6: ponte de migração controlada criada; nenhuma substituição em massa dos 61 `glBegin`.
- [x] FASE 7: infraestrutura de GPU skinning criada e caminho BMD moderno auditado.

### Correções aplicadas

- [x] Contrato `Render/Shader/GPUSkinningShader.h` adicionado com layout de posição, normal, UV, bone indices, bone weights e BoneUBO slot 2.
- [x] Shader de referência agora inclui skinning de posição **e normal**.
- [x] Corrigido bug no `New_RenderBMD.cpp`: teste de alpha do caminho `RENDER_TEXTURE` comparava incorretamente `m_FlagRender` com `0.99f`; agora usa `m_isAlpha < 0.99f`.
- [x] Nenhuma ativação global do GPU skinning foi feita.
- [x] Fallback legado permanece preservado.

### Achados importantes

- O `Main.vcxproj` contém os `.cpp` tradicionais, mas os novos arquivos de infraestrutura FASE 7 ainda não foram adicionados ao projeto; isso deve ser feito somente junto da integração física.
- O caminho `New_ModelBMD`/`New_RenderBMD` ainda envia uma palette por uniform; BoneUBO/std140 ainda não está conectado.
- O BMD atual fornece um `Vertex_t::Node` por vértice. Portanto, a representação de quatro influências deve continuar usando uma única influência com peso `1.0` até que exista dado real de pesos no asset.
- Os shaders `Data\\Effect\\VBO\\*.vs/*.fs` são carregados externamente pelo cliente e não foram encontrados no repositório fonte; a nova declaração em `Render/Shader/GPUSkinningShader.h` é um contrato de referência, não substitui esses assets runtime.

### Próximos passos da FASE 7

- [ ] Integrar os novos tipos no VAO real de BMD.
- [ ] Criar transporte `BoneIndex + BoneWeight` mantendo compatibilidade com `Vertex_t::Node`.
- [ ] Migrar palette do caminho moderno de uniform para `BoneUBO/std140`.
- [ ] Conectar `BoneTransform` → palette sem duplicar animação CPU.
- [ ] Integrar shader runtime real com posição + normal skinning.
- [ ] Adicionar somente os `.cpp` necessários ao `Main.vcxproj`.
- [ ] Compilar `Global Release|x86` no ambiente local.
- [ ] Validar animação visual e fallback.
- [ ] Só depois promover GPU skinning.

### Regra de segurança

Não ativar globalmente `jdk_shader_local330` nem remover `CoreGLCompat` durante a FASE 7. O caminho moderno deve ser validado isoladamente antes de qualquer migração ampla.

---

## 2026-09-07 — Sessão 5 (FASE 7 — infraestrutura e ponte BMD)

- [x] Limite de `200` bones alinhado com `MAX_BONES`/BoneUBO.
- [x] `BonePalette` CPU 3x4.
- [x] Conversão explícita 3x4 → 4x4.
- [x] Proteção de índices.
- [x] Normalização segura de até 4 pesos.
- [x] Layout preparado para posição, normal, UV, bone indices e weights.
- [x] Ponte `BMDGpuSkinningBridge` criada.
- [x] Representação atual do BMD preservada sem inventar pesos.
- [x] Fallback legado preservado.

## 2026-09-07 — Sessão 4 (FASE 6 — Core Profile / migração controlada)

- [x] `Render/Immediate/LegacyImmediateAdapter.h` criado.
- [x] ImmediateRenderer integrado como camada de conversão.
- [x] `GL_QUADS` convertido para `GL_TRIANGLES`.
- [x] Adapter mantém submissão física no `CoreGLCompat`.
- [x] Migração individual preparada; substituição em massa não feita.

## 2026-09-07 — Sessão 3 (FASE 5 — ImmediateRenderer)

- [x] `Render/Immediate/ImmediateRenderer.h` criado.
- [x] Staging de posição, cor, UV e normal.
- [x] Conversão de primitivas legadas para batches.
- [x] API preparada para OpenGL/Vulkan.

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
- FASE 4: concluída como fundação
- FASE 5: concluída como camada de conversão
- FASE 6: ponte concluída; migração física pendente
- FASE 7: infraestrutura + auditoria + correções seguras; integração física pendente
