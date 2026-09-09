# 15 — Model resource bindings exatos

Status: **CONFIRMADO por assembly x64 + Diligent Engine v2.5.4**.

Este documento descreve a separação real entre recursos estáticos do PSO e recursos mutáveis do SRB no renderer de modelos do NextMU.

## Funções principais

- model render/resource binding: `0x14008D530..0x14008DF0A` (aprox. pelo `.pdata`)
- resource-layout builder: `0x14009F600..0x1400A075B`

## Métodos Diligent identificados pela vtable v2.5.4

- `IPipelineState + 0x68` = `GetStaticVariableByName(ShaderType, Name)`
- `IShaderResourceBinding + 0x38` = `GetVariableByName(ShaderType, Name)`
- `IShaderResourceVariable + 0x20` = `Set(IDeviceObject*, flags)`

## Static bindings do Model PSO

| Recurso | Shader stage | Tipo | Evidência |
|---|---:|---|---|
| `cbCameraAttribs` | VS (`1`) | STATIC | GetStaticVariableByName + Set |
| `cbLightAttribs` | VS (`1`) + PS (`2`) | STATIC | duas chamadas, mesmo recurso |
| `ModelViewProj` | VS | STATIC | string VA `0x140484C28`; buffer global `0x1409CA620` |
| `g_SkeletonTexture` | VS | STATIC | string VA `0x140484C38`; texture view obtida antes do Set |
| `ModelSettings` | VS + PS | STATIC | string VA `0x140484C50`; buffer global `0x1409CA628` |

### Offsets de interesse no render

```text
0x14008D99E ModelViewProj / VS
0x14008D9AA GetStaticVariableByName
0x14008D9BC buffer = [0x1409CA620]

0x14008D9D0 g_SkeletonTexture / VS
0x14008D9DC GetStaticVariableByName

0x14008DA14 ModelSettings / VS
0x14008DA20 GetStaticVariableByName
0x14008DA32 buffer = [0x1409CA628]

0x14008DA46 ModelSettings / PS
0x14008DA52 GetStaticVariableByName
0x14008DA64 buffer = [0x1409CA628]
```

## Mutable bindings por SRB

| Recurso | Stage | Tipo |
|---|---:|---|
| `g_VertexTexture` | VS | MUTABLE |
| `g_Texture` | PS | MUTABLE |
| `g_tex2DShadowMap` | PS | MUTABLE |
| `g_tex2DFilterableShadowMap` | PS | MUTABLE |

No draw path, o renderer obtém o `IShaderResourceBinding` da combinação de programa/material e resolve essas variáveis por nome. Texturas por mesh/material são ligadas no SRB; os buffers globais ficam no PSO.

## Shadow binding

O renderer escolhe entre:

```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

conforme o modo de shadow retornado pelo helper do renderer. O recurso selecionado é ligado no PS SRB via `GetVariableByName()` + `Set()`.

## Arquitetura recuperada

```text
PSO (global / raro de mudar)
 ├─ cbCameraAttribs
 ├─ cbLightAttribs
 ├─ ModelViewProj
 ├─ g_SkeletonTexture
 └─ ModelSettings

SRB (por material/mesh)
 ├─ g_VertexTexture
 ├─ g_Texture
 └─ shadow texture
```

## Por que isso é bom para nosso MU

- reduz churn de descriptors no Vulkan;
- evita reenviar câmera/luz/settings por mesh;
- separa estado de frame/modelo de estado de material;
- permite cache de SRBs por material/programa;
- funciona como contrato comum para Vulkan e OpenGL 4.6.

## Limites

- tamanhos exatos de `ModelViewProj` e `ModelSettings` ainda não foram provados;
- formato físico da skeleton texture ainda precisa de análise do criador/upload;
- a estrutura exata da chave de cache de SRB/PSO ainda está em análise.
