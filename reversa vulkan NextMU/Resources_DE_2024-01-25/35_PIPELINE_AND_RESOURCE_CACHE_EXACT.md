# 35 — Pipeline / Resource cache exato

Status: **CONFIRMADO POR SOURCE + BINÁRIO**.

## Pipeline cache

O NextMU mantém:

```cpp
map<NFixedPipelineHash,
    map<NDynamicPipelineHash, NPipelineState>> Pipelines;
```

`GetPipelineState()` procura primeiro o hash fixo e depois o hash dinâmico. Em miss chama `CreatePipelineState()`; em hit reaproveita o PSO.

## Fixed key

`NFixedPipelineState` contém:

```text
CombinedShader
RTVFormat
DSVFormat
```

e compacta os três em `NFixedPipelineHash`.

## Dynamic key

`NDynamicPipelineState` compacta em 64 bits:

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

## Criação do PSO

`CreatePipelineState` preenche:

```text
RasterizerDesc
BlendDesc
DepthStencilDesc
InputLayout
VS / PS
ResourceLayout ou ResourceSignatures
RTV/DSV formats
```

Depois chama:

```cpp
IRenderDevice::CreateGraphicsPipelineState(...)
```

Cada novo pipeline recebe `PipelineIdCache++`.

## PipelineStateInfo

O cache registra pelo menos:

```text
Shader
DepthWrite
BlendEnable
SrcBlend
DestBlend
BlendHash
```

Esses dados são usados pelo `NRenderManager` para classificação/ordenação.

## Resource layouts

### mesh
Static:
```text
cbCameraAttribs
cbLightAttribs
ModelViewProj
g_SkeletonTexture
ModelSettings
```
Mutable:
```text
g_VertexTexture
g_Texture
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

### terrain / grass
Static:
```text
cbCameraAttribs
cbLightAttribs
g_HeightTexture
g_LightTexture
g_NormalTexture
g_MappingTexture
g_UVTexture
g_AttributesTexture
TerrainSettings
g_Textures
```
Mutable shadow resources:
```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

### particle
Static:
```text
cbCameraAttribs
ParticleSettings
```
Mutable:
```text
g_Texture
```

### joint
Static:
```text
cbCameraAttribs
JointSettings
```
Mutable:
```text
g_Texture
```

### bbox
```text
cbCameraAttribs
```

### RmlUI
Color:
```text
cbRmlAttribs
```
Texture:
```text
cbRmlAttribs
g_Texture (mutable)
```

## Recomendação para nosso frontend

Use a mesma decomposição:

```text
FixedPipelineKey  = shader + render-target formats
DynamicStateKey   = raster/depth/stencil/blend
ResourceKey       = IDs dos recursos mutáveis
```

OpenGL pode mapear a chave para program/state cache; Vulkan para VkPipeline/descriptor bindings.
