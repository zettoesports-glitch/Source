# 23 — Model draw path exato

Status: **CONFIRMADO** pelo fluxo de `Objects::Render`, `Characters::Render` e renderer compartilhado.

## Convergência Objects / Characters

```text
Objects::Render      0x1400A68B0
Characters::Render   0x140068350
          \\          /
           \\        /
             0x14008D2F0
                 ↓
             per mesh
             0x14008D6B0
```

Isso prova que Objects e Characters não mantêm dois backends de modelo independentes. Ambos alimentam o mesmo frontend/model renderer.

## Recursos estáticos do PSO

Inicialização lazy do pipeline resolve por nome e fixa:

```text
cbCameraAttribs     VS
cbLightAttribs      VS
cbLightAttribs      PS
ModelViewProj       VS
g_SkeletonTexture   VS
ModelSettings       VS
ModelSettings       PS
```

São acessados via `IPipelineState::GetStaticVariableByName()` e `IShaderResourceVariable::Set()`.

## Recursos mutáveis do SRB

Por material/mesh:

```text
g_VertexTexture              VS
g_Texture                    PS
g_tex2DShadowMap             PS
ou
g_tex2DFilterableShadowMap   PS
```

São resolvidos através do `IShaderResourceBinding`.

## Uniform arenas

Por draw/mesh são obtidos slices reutilizáveis:

```text
ModelViewProj  128 bytes
ModelSettings   96 bytes
```

Os uploads são emitidos como comandos, não executados inline.

## Fluxo reconstruído por mesh

```text
1. resolver estado/material do model.json
2. selecionar/criar wrapper de programa/PSO
3. inicializar static resources se necessário
4. obter slice ModelViewProj
5. copiar 2 matrizes (2 * 64 B)
6. enfileirar upload do slice
7. obter slice ModelSettings
8. preencher 96 B
9. enfileirar upload
10. selecionar mutable resources do SRB
11. enfileirar SetPipelineState
12. enfileirar SetVertexBuffers / index quando aplicável
13. enfileirar CommitShaderResources
14. enfileirar Draw/DrawIndexed
15. central replay executa e faz transitions em lote
```

## Shadow binding

A escolha entre:

```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

é dinâmica. Portanto o contrato do model renderer já prevê mais de uma técnica/representação de shadow map sem trocar o frontend inteiro.

## Implicação para nosso ModernBMD

A arquitetura recomendada é separar:

```text
BMD/OBJECT gameplay state
        ↓
ModelDrawPacket
        ↓
RenderStateKey + resource handles
        ↓
DynamicUniformArena
        ↓
RenderCommandBuffer
        ↓
GL4.6+ / Vulkan backend
```

Isso evita estado por instância vazando entre Hero/remote player/NPC/monster/bot e é muito mais robusto do que chamar GL diretamente de `ZzzBMD.cpp`.
