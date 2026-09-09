# 33 — Terrain, Grass e Water exatos

Status: **CONFIRMADO — source histórica + binário**.

## TerrainSettings

```cpp
struct TerrainSettings
{
    float WaterMove;
    float WindScale;
    float WindSpeed;
    float Dummy;
}; // 16 B
```

Atualização por frame:

```cpp
WaterMove = mod(WorldTime, WaterModulus) * WaterMultiplier;
WindScale = 10.0f;
WindSpeed = mod(WorldTime, WindModulus) * WindMultiplier;
Dummy = 0.0f;
```

`WaterModulus`, `WaterMultiplier`, `WindModulus` e `WindMultiplier` vêm de `terrain.json`.

O UBO é atualizado com `MAP_WRITE | MAP_FLAG_DISCARD`.

## Vertex compactado

A build usa `NEXTMU_COMPRESSED_TERRAIN = 1`.

```cpp
struct NTerrainVertex
{
    uint8_t X;
    uint8_t Y;
    uint8_t RX;
    uint8_t RY;
}; // 4 B
```

Semântica:

- `X,Y`: canto local da célula;
- `RX,RY`: coordenada da célula no terrain.

Cada célula gera 6 vertices diretamente:

```text
(0,0) (1,0) (1,1)
(0,0) (1,1) (0,1)
```

O shader reconstrói posição/atributos usando textures do terrain; não há posição float3 por vértice.

## Textures/formatos confirmados

Entre os recursos gerados/carregados:

```text
HeightmapTexture   → R32_FLOAT
NormalTexture      → RGBA16_FLOAT
LightTexture
a MappingTexture
UVTexture
AttributesTexture
Textures / texture array
GrassUVTexture
GrassTextures
```

Bindings do Terrain:

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

No modo normal pode adicionar shadow resource via SRB:

```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

## Programs

Terrain:

```text
normal    → terrain
shadowmap → terrain_shadow
```

Grass:

```text
normal    → grass
shadowmap → grass_shadow
```

## PSO do Terrain

Fixed state:

```text
CombinedShader = terrain / terrain_shadow
RTVFormat      = render target color format
DSVFormat      = render target depth format
```

Dynamic state:

```text
Normal    → DefaultDynamicPipelineState
ShadowMap → DefaultShadowDynamicPipelineState
```

Classificação de draw:

```text
NRenderClassify::Opaque
Index = 1
```

Para cada linha visível do terrain:

```text
Draw(range.End - range.Start, StartVertex = range.Start)
```

Portanto o culling CPU produz ranges/linhas visíveis e o renderer só submete essas faixas do vertex buffer.

## Grass

Grass usa o mesmo desenho de ranges do terrain e resources equivalentes, trocando `UVTexture/Textures` por `GrassUVTexture/GrassTextures`.

Estado normal confirmado:

```text
CullMode   = NONE
AlphaWrite = false
DepthWrite = false
SrcBlend   = SRC_ALPHA
DstBlend   = INV_SRC_ALPHA
SrcBlendAlpha = SRC_ALPHA
DstBlendAlpha = INV_SRC_ALPHA
```

Classificação:

```text
NRenderClassify::PreAlpha
```

No ShadowMap usa `DefaultShadowDynamicPipelineState`.

## Water

Nesta build Water não aparece como um backend/mesh renderer completamente separado do terrain. O movimento da água faz parte dos dados globais `TerrainSettings` via `WaterMove`, e a configuração vem de `terrain.json` (`water.mod`, `water.mul`).

Isto é diferente do Main OpenGL 4.6 analisado anteriormente, onde existe um `RendererWaterGL46` dedicado. Para nosso projeto podemos escolher a abordagem melhor, mantendo o frontend de material/terrain independente do backend.

## Aplicação ao nosso MU

Boas ideias para portar:

- Terrain vertex de 4 B;
- height/normal em textures GPU;
- culling em ranges por linha;
- draw apenas das faixas visíveis;
- terrain Opaque separado de grass PreAlpha;
- configuração de water/wind por dados;
- mesmo frontend para normal/shadow pass;
- cache de PSO/SRB compartilhado com Model/Effects.
