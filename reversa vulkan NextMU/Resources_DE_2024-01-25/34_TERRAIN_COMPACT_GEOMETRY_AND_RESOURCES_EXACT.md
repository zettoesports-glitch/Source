# 34 — Terrain: geometria compacta, resources e animação

Status: **CONFIRMADO-SOURCE / consistente com binário**.

## Vertex de 4 bytes

```cpp
struct NTerrainVertex
{
    uint8_t X, Y;
    uint8_t RX, RY;
};
```

InputLayout:

```text
ATTRIB0 uint8x2 X/Y   offset 0 stride 4
ATTRIB1 uint8x2 RX/RY offset 2 stride 4
```

## Significado exato

Para cada célula `(x,y)`, o NextMU gera os seis vértices de dois triângulos:

```text
(0,0,x,y)
(1,0,x,y)
(1,1,x,y)
(0,0,x,y)
(1,1,x,y)
(0,1,x,y)
```

Logo:

```text
RX,RY = coordenada da célula do terrain
X,Y   = corner local 0/1 dentro da célula
```

A posição/altura completa é reconstruída com dados de terrain e textures no shader.

## Buffer global

```text
TerrainVertices[TerrainSize * TerrainSize * 6]
```

É uma triangle list pré-gerada, extremamente compacta.

## TerrainSettings — 16 bytes

```cpp
struct TerrainSettings
{
    float WaterMove;
    float WindScale;
    float WindSpeed;
    float Dummy;
};
```

Por frame:

```text
WaterMove = mod(WorldTime, WaterModulus) * WaterMultiplier
WindScale = 10.0
WindSpeed = mod(WorldTime, WindModulus) * WindMultiplier
Dummy = 0
```

O UBO é atualizado via Map/Discard.

## Resources do Terrain

STATIC:

```text
VS     cbCameraAttribs
VS+PS  cbLightAttribs
VS     g_HeightTexture
VS     g_LightTexture
VS     g_NormalTexture
VS     g_MappingTexture
VS     g_UVTexture
VS     g_AttributesTexture
VS     TerrainSettings
PS     g_Textures
```

MUTABLE:

```text
PS g_tex2DShadowMap
PS g_tex2DFilterableShadowMap
```

Grass usa o mesmo conjunto estrutural, mas troca `g_UVTexture`/`g_Textures` pelas variantes de Grass.

## Heightmap GPU

GPU texture:

```text
Type       2D
Size       TerrainSize x TerrainSize
Format     R32_FLOAT
Usage      IMMUTABLE
Bind       SHADER_RESOURCE
```

Compatibilidade de arquivos:

- OZB legado;
- caso V1 `BM8` / 66616 bytes / dados a partir de 1080;
- bitmap 8-bit;
- bitmap 24-bit, RGB convertido para altura e deslocado por `-500`.

## Normal map

Normal é calculada na CPU e empacotada em half-float:

```text
RGBA16_FLOAT
TerrainSize x TerrainSize
```

A texture é consumida pelo vertex shader.

## Terrain attributes

Armazenados como `uint16`, GPU format `R16_UINT`.

```text
SafeZone      bit 0
Character     bit 1
NoMove        bit 2
NoGround      bit 3
Water         bit 4
Action        bit 5
Height        bit 6
CameraUp      bit 7
NoAttackZone  bit 8
```

## Culling + draws

`CullingTree->GenerateRenderRanges(RenderSettings)` produz linhas/ranges visíveis.

O render faz um `Draw` por range:

```text
NumVertices         = range.End - range.Start
StartVertexLocation = range.Start
NumInstances        = 1
Flags               = VERIFY_ALL
```

Terrain e Grass usam os ranges de visibilidade em vez de desenhar todo o mapa.

## Navmesh

Usa Detour. O terrain cria **um `dtNavMeshQuery` por thread**, de acordo com `MUThreadsManager::GetThreadsCount()`.

## Valor para nosso renderer

Este desenho é excelente para MU:

```text
4-byte terrain vertex
+ GPU height/normal/light/mapping textures
+ CPU culling tree
+ contiguous visible ranges
+ one draw per range
```

Mantém bandwidth de vertex muito baixo e funciona igualmente bem em OpenGL 4.6+ e Vulkan.
