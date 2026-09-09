# 31 — Terrain pipeline do NextMU

Status: **CONFIRMADO onde indicado**. Nomes sem prova semântica continuam descritos como variantes/pass, não como hipótese de efeito.

Função principal analisada: aproximadamente `0x1400516C0`.

## Vertex format extremamente compacto

O InputLayout `terrain` usa apenas **4 bytes por vértice**:

```text
ATTRIB0 uint8x2 offset 0 stride 4, unnormalized
ATTRIB1 uint8x2 offset 2 stride 4, unnormalized
```

Logo o terreno não envia `float3 position/normal/uv` completos. O shader reconstrói os dados usando texturas do terrain.

## Recursos do Terrain

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
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

VAs confirmados em x64:

```text
0x140482090 cbCameraAttribs
0x1404820A0 cbLightAttribs
0x1404820B0 g_HeightTexture
0x1404820C0 g_LightTexture
0x1404820D0 g_NormalTexture
0x1404820E0 g_MappingTexture
0x1404820F8 g_UVTexture
0x140482108 g_AttributesTexture
0x140482120 TerrainSettings
0x140482130 g_Textures
0x140482140 g_tex2DFilterableShadowMap
0x1405F36A8 g_tex2DShadowMap
```

## Static resources

Na inicialização lazy do renderer, o PSO usa `IPipelineState::GetStaticVariableByName` e depois `ShaderResourceVariable::Set`.

Confirmado:

```text
cbCameraAttribs     VS
cbLightAttribs      VS + PS
g_HeightTexture     VS
g_LightTexture      VS
g_NormalTexture     VS
g_MappingTexture    VS
g_UVTexture         VS
g_AttributesTexture VS
TerrainSettings     VS
g_Textures          PS
```

As texturas são convertidas para shader-resource views através do default texture view.

## Resource variable types

A criação do resource layout no setup central confirma:

```text
Height/Light/Normal/Mapping/UV/Attributes = STATIC
TerrainSettings                            = STATIC
g_Textures                                 = STATIC
g_tex2DShadowMap                           = MUTABLE
g_tex2DFilterableShadowMap                 = MUTABLE
```

Isso evita recriar bindings invariantes e permite trocar apenas a shadow texture/variant por SRB.

## Shadow binding

O runtime consulta o modo de sombra através do helper `0x14007E750`.

Quando o resultado seleciona o shadow map comum:

```text
SRB variable = g_tex2DShadowMap
source       = shadow object +0x18
```

Na alternativa filtrável:

```text
SRB variable = g_tex2DFilterableShadowMap
source       = shadow object +0x38
```

Após inicializar esses resources, o cache object marca `byte +0x08` como inicializado.

## PSO/SRB cache

Terrain chama o cache central `0x14004A0E0` em múltiplos passes/variantes, igual ao Model renderer.

## Draw path

O runtime enfileira:

```text
SetVertexBuffers       -> 0x14009A900
SetPipelineState       -> 0x14009A860
CommitShaderResources  -> 0x140099F70
Draw                    -> 0x14009A020
```

Portanto o terrain usa **Draw não-indexado** nesses passes analisados.

## Segmentação

Os draws percorrem uma tabela de segmentos em torno de `terrain object +0x98`.
Cada entrada fornece pelo menos um início e fim; o draw é construído com:

```text
vertex_count = end - start
start_vertex = start
```

Isto permite renderizar apenas blocos/faixas selecionadas do terrain.

## Programas e configuração

Strings confirmadas:

```text
terrain
terrain.json
terrain program not found ({}, {})
grass_program
terrain grass program not found ({}, {})
height_multiplier
heightmap
lightmap
light_position
grass_textures
mappings
attributes
models
objects
water
wind
scale
```

O objeto mantém pelo menos dois IDs de programas em `+0x20` e `+0x22`. A função possui dois blocos/passes muito semelhantes que usam variantes desses programas; a semântica exata de cada variante ainda está sendo ligada ao JSON/shader antes de receber nome definitivo.

`grass_program` é resolvido separadamente.

## Implicação para nosso MU

A referência sugere uma arquitetura eficiente:

```text
TerrainVertex4
   ↓
static terrain textures/settings
   ↓
segment selection
   ↓
PSO/SRB cache
   ↓
command queue
   ↓
GL4.6+ / Vulkan
```

Especialmente valiosos para nosso renderer:
- vertex de terrain compacto;
- resources invariantes static;
- shadows mutable;
- segment/chunk draws;
- command batching;
- backend-independent frontend.