# 34 — Shader/resource system do NextMU

Status: **SOURCE-CORRELATED** com `mu_resourcesmanager.cpp` e `t_graphics_pipelineresources.cpp` históricos, consistente com strings/xrefs do EXE.

## Fonte única de shader

O cliente carrega vertex/pixel shader de arquivos externos e cria os shaders Diligent com:

```text
SourceLanguage = HLSL
UseCombinedTextureSamplers = true
```

Isso permite manter um shader HLSL comum e deixar a Diligent transformá-lo para o backend ativo.

### OpenGL

Antes da compilação GL/GLES o cliente normaliza tipos:

```text
half4 -> float4
half3 -> float3
half2 -> float2
half  -> float
```

### Vulkan

O backend Vulkan da Diligent v2.5.4 contém a cadeia HLSL/GLSL -> SPIR-V e reflection já identificada na DLL.

## `resources.json`

O loader principal lê:

```text
data/resources.json
```

Seções reconhecidas:

```text
attachments
shaders
textures
models
```

Cada shader configurado por JSON possui:

```text
id
vertex
fragment
resource_id
macros[]
```

Os macros adicionais aceitam tipos:

```text
integer
float
boolean
string
```

## Macros globais injetadas em todos os programas

```text
SKELETON_TEXTURE_WIDTH
SKELETON_TEXTURE_HEIGHT
USE_SHADOW
SHADOW_MODE
SHADOW_FILTER_SIZE
FILTER_ACROSS_CASCADES
BEST_CASCADE_SEARCH
```

Logo variantes de shadow/skeleton são compiladas por macro, não por um shader completamente separado para cada combinação.

## `resource_id`

O mesmo `resource_id` seleciona:

```text
InputLayout
PipelineResource layout
```

Fluxo:

```text
resources.json shader
   -> resource_id
      -> GetInputLayout(resource_id)
      -> GetPipelineResource(resource_id)
      -> CreateShader VS/PS
      -> RegisterShader
```

## Resource layouts registrados

### mesh

Static:
```text
cbCameraAttribs        VS
cbLightAttribs         VS+PS
ModelViewProj          VS
g_SkeletonTexture      VS
ModelSettings          VS+PS
```

Mutable:
```text
g_VertexTexture        VS
g_Texture              PS
g_tex2DShadowMap       PS
g_tex2DFilterableShadowMap PS
```

### terrain

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

Mutable:
```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

### grass

Usa essencialmente o mesmo contrato de terrain; em runtime o objeto Terrain liga `GrassUVTexture` no slot `g_UVTexture` e `GrassTextures` no `g_Textures`.

### joint

```text
cbCameraAttribs  static VS
JointSettings    static PS
g_Texture        mutable PS
```

### particle

```text
cbCameraAttribs  static VS
ParticleSettings static PS
g_Texture        mutable PS
```

### bbox

```text
cbCameraAttribs static VS
```

### RmlUI

Color:
```text
cbRmlAttribs static VS+PS
```

Texture:
```text
cbRmlAttribs static VS+PS
g_Texture    mutable PS
```

## Estratégia para nosso MU

Recomendado manter exatamente a ideia, sem obrigatoriamente depender da Diligent:

```text
HLSL comum
+ macros de feature
+ ResourceLayout declarativo
+ backend compiler
   GL4.6+: HLSL->SPIR-V/GLSL ou shaders equivalentes validados
   Vulkan: HLSL->SPIR-V
```

Isso reduz divergência entre OpenGL e Vulkan e facilita updates futuros.
