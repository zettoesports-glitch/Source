# 34 — PSO cache e RenderState exatos

Status: **CONFIRMADO — source histórica de 25/01/2024 + comportamento observado no EXE**.

## Cache em dois níveis

O NextMU usa:

```cpp
using DynamicPipelineMap = std::map<NDynamicPipelineHash, NPipelineState>;
using FixedPipelineMap   = std::map<NFixedPipelineHash, DynamicPipelineMap>;
```

Fluxo:

```text
FixedHash
  └─ DynamicHash
       └─ NPipelineState
```

`GetPipelineState(fixed, dynamic)`:

1. procura o `FixedHash`;
2. procura o `DynamicHash` dentro dele;
3. cria PSO apenas em cache miss;
4. retorna o PSO já existente nas próximas chamadas.

Cada PSO novo recebe um `PipelineIdCache++` usado também pelo cache de SRBs.

## Fixed key

`NFixedPipelineState` produz um hash compacto contendo:

```text
CombinedShader
RTVFormat
DSVFormat
```

A ideia é separar propriedades que alteram shader/render target das propriedades raster/depth/blend.

## Dynamic key

`NDynamicPipelineState` é comprimido em `mu_uint64` e inclui:

```text
CullMode
EnableScissors
ColorWrite
AlphaWrite
DepthWrite
DepthFunc
StencilEnable
StencilReadMask
StencilFailOp
StencilDepthFailOp
StencilPassOp
StencilFunc
SrcBlend
DestBlend
SrcBlendAlpha
DestBlendAlpha
BlendOp
BlendOpAlpha
```

O `GetHash()` monta esses campos via shifts calculados pelo número de bits necessário para cada enum.

## Criação do PSO

Em cache miss, `CreatePipelineState()` monta `GraphicsPipelineStateCreateInfo`:

### Rasterizer

```text
CullMode = dynamicState.CullMode
```

### Blend

Blend é ativado quando Src/Dst/BlendOp estão definidos.

```text
SrcBlend
DestBlend
BlendOp
SrcBlendAlpha
DestBlendAlpha
BlendOpAlpha
```

Write mask é construído separadamente:

```text
ColorWrite → RGB
AlphaWrite → A
```

### Depth/stencil

```text
DepthWriteEnable
DepthEnable = DepthFunc != UNKNOWN
DepthFunc
StencilEnable
StencilReadMask
StencilWriteMask = 0xFF
front/back stencil ops/function
```

### Shader/input layout

O PSO usa o programa resolvido por `CombinedShader`:

```text
shader->Layout
shader->Vertex
shader->Pixel
```

Resources podem vir de:

```text
Pipeline ResourceLayout implícito
ou
Pipeline Resource Signatures explícitas
```

### Render targets

```text
RTVFormat conhecido → 1 render target
RTVFormat UNKNOWN   → color write mask NONE, zero RTs
DSVFormat           → fixedState.DSVFormat
```

Isso permite reutilizar o mesmo mecanismo no shadow pass sem color target.

## Metadata guardada por PSO

`NPipelineState` preserva:

```text
Id
Shader
DepthWrite
BlendEnable
SrcBlend
DestBlend
BlendHash
Pipeline object
StaticInitialized
```

Em debug também guarda os fixed/dynamic states completos.

## Shadow normalization

`NormalizeShadowRenderState()` força:

```text
CullMode = NONE
ColorWrite = false
AlphaWrite = false
Stencil disabled/default
Blend factors = UNDEFINED
Blend ops = ADD
```

preservando os campos relevantes de depth quando apropriado.

## SRB cache ligado ao PSO

A chave de resource binding é baseada em:

```text
PipelineState.Id
+ quantidade de resources
+ lista ordenada de ResourceId
```

Assim o PSO e seus descriptors/resources são reaproveitados juntos.

## Aplicação ao nosso renderer

Este desenho é um dos melhores candidatos para portar praticamente igual em conceito:

```text
RenderStateDesc
  ↓
FixedKey + DynamicKey
  ↓
PipelineCache
  ├─ OpenGL46StateObject/Program binding
  └─ Vulkan VkPipeline/descriptor layout
```

No OpenGL 4.6, não existe `VkPipeline` monolítico, mas a mesma key pode alimentar um state cache de program/blend/depth/cull. No Vulkan, a key cria/reusa o PSO real.
