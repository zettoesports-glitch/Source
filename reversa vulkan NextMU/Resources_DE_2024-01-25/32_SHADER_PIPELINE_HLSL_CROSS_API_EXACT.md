# 32 — Pipeline de shaders HLSL cross-API

Status: **CONFIRMADO-SOURCE / consistente com backend fornecido**.

## Fonte de configuração

O cliente lê:

```text
data/resources.json
```

Cada programa de shader contém:

```text
id
vertex
fragment
resource_id
macros[]
```

Macros customizadas podem ser `integer`, `float`, `boolean` ou `string`.

## Macros globais injetadas

```text
SKELETON_TEXTURE_WIDTH
SKELETON_TEXTURE_HEIGHT
USE_SHADOW
SHADOW_MODE
SHADOW_FILTER_SIZE
FILTER_ACROSS_CASCADES
BEST_CASCADE_SEARCH
```

## Compilação

O cliente lê vertex e pixel shader como texto e cria:

```cpp
ShaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
ShaderCreateInfo.Desc.UseCombinedTextureSamplers = true;
```

O mesmo HLSL alimenta todos os backends.

### Vulkan

```text
HLSL
 -> Diligent shader compiler
 -> SPIR-V
 -> Vulkan shader module/pipeline
```

A `GraphicsEngineVk_*r.dll` fornecida inclui o backend Vulkan, glslang/SPIR-V tooling usado pela geração da Diligent correspondente.

### OpenGL

Antes de enviar o HLSL para Diligent, o NextMU normaliza tipos half:

```text
half4 -> float4
half3 -> float3
half2 -> float2
half  -> float
```

Depois:

```text
HLSL -> Diligent -> GLSL/OpenGL
```

### D3D11/D3D12

O mesmo source HLSL é usado no backend Direct3D.

## Input layout e resources por `resource_id`

O `resource_id` liga cada shader ao contrato do frontend:

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

Cada contrato define:

- InputLayout;
- static shader variables;
- mutable resources;
- immutable samplers quando configurados.

## Arquitetura recomendada para nosso MU

Manter uma linguagem-fonte única de shader é muito mais sustentável que manter GLSL e Vulkan/SPIR-V manualmente divergentes.

Opções futuras:

```text
HLSL-first -> SPIR-V + GLSL
ou
GLSL/Slang-first -> SPIR-V + OpenGL
```

O requisito principal é manter uma interface única de:

```text
vertex attributes
Frame/Camera data
material data
texture bindings
bones/skeleton
shadow resources
```

## Benefício

Uma correção visual em Model/Particle/Terrain pode ser aplicada uma vez e validada em OpenGL 4.6+ e Vulkan sem duplicar a lógica de material.
