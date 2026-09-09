# 35 — Shader loader e resource pipeline exatos

Status: **CONFIRMADO — source histórica de 25/01/2024 + strings/arquitetura observadas no binário**.

## Fonte dos programas

O cliente carrega `data/resources.json` e lê a seção `shaders`.

Cada programa define:

```text
id
vertex
fragment
resource_id
macros[] (opcional)
```

`LoadPrograms()` chama `LoadProgram(id, vertex, fragment, resource_id, macros)`.

## Linguagem fonte

A build usa:

```cpp
createInfo.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
createInfo.Desc.UseCombinedTextureSamplers = true;
```

Portanto a fonte autoral do jogo é HLSL, e a Diligent converte/compila conforme backend.

## Backends

O mesmo HLSL é usado para:

```text
D3D11
Vulkan
D3D12
OpenGL
```

Para OpenGL/GLES o NextMU faz um pré-processamento simples:

```text
half4 → float4
half3 → float3
half2 → float2
half  → float
```

antes da compilação.

No Vulkan, a Diligent v2.5.4 usa seu pipeline de compilação/conversão para gerar SPIR-V.

## Macros globais injetadas

Antes de compilar cada programa, o NextMU injeta:

```text
SKELETON_TEXTURE_WIDTH
SKELETON_TEXTURE_HEIGHT
USE_SHADOW
SHADOW_MODE
SHADOW_FILTER_SIZE
FILTER_ACROSS_CASCADES
BEST_CASCADE_SEARCH
```

Além disso `resources.json` pode adicionar macros por programa com tipos:

```text
integer
float
boolean
string
```

## InputLayout + ResourceLayout

O `resource_id` seleciona simultaneamente:

```cpp
settings.InputLayout = GetInputLayout(resourceId);
settings.Resource    = GetPipelineResource(resourceId);
```

Ou seja, o contrato de shader não é hardcoded no caller: um mesmo ID seleciona layout de vértice e resource variables.

IDs confirmados:

```text
mesh
terrain
grass
joint
particle
bbox
rmlui_color
rmlui_texture
```

## Program registration

Depois de criar VS e PS:

```text
NCombinedShader
├─ Layout
├─ Vertex
├─ Pixel
├─ ResourceSignatures
└─ Resource
```

é registrado e recebe um `mu_shader` numérico. `MUResourcesManager::GetProgram(id)` retorna esse handle.

## Resource variables exatas

### mesh

Static:
```text
VS      cbCameraAttribs
VS+PS   cbLightAttribs
VS      ModelViewProj
VS      g_SkeletonTexture
VS+PS   ModelSettings
```

Mutable:
```text
VS      g_VertexTexture
PS      g_Texture
PS      g_tex2DShadowMap
PS      g_tex2DFilterableShadowMap
```

### terrain / grass

Static:
```text
VS      cbCameraAttribs
VS+PS   cbLightAttribs
VS      g_HeightTexture
VS      g_LightTexture
VS      g_NormalTexture
VS      g_MappingTexture
VS      g_UVTexture
VS      g_AttributesTexture
VS      TerrainSettings
PS      g_Textures
```

Mutable:
```text
PS      g_tex2DShadowMap
PS      g_tex2DFilterableShadowMap
```

### joint

```text
VS static       cbCameraAttribs
PS static       JointSettings
PS mutable      g_Texture
```

### particle

```text
VS static       cbCameraAttribs
PS static       ParticleSettings
PS mutable      g_Texture
```

### bbox

```text
VS static       cbCameraAttribs
```

### rmlui_color

```text
VS+PS static    cbRmlAttribs
```

### rmlui_texture

```text
VS+PS static    cbRmlAttribs
PS mutable      g_Texture
```

## Estratégia para nosso renderer

O ponto mais forte para copiar é o contrato data-driven:

```text
Program definition
  ├─ shader source(s)
  ├─ compile macros
  ├─ input layout id
  └─ resource layout id
        ↓
backend compiler
        ↓
OpenGL 4.6+ program / Vulkan pipeline shader modules
```

Isso reduz divergência entre backends e facilita updates futuros.

## Limitação atual

O pacote `Resources_DE_2024-01-25.zip` analisado contém executáveis/DLLs de engine, mas não contém o diretório `data/` com `resources.json` e os HLSL autorais. Os contratos e loader estão completos; os arquivos HLSL precisam ser obtidos de um pacote de resources correspondente ou reconstruídos/identificados em outro mirror.
