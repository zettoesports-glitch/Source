# 33 — Terrain / Grass / Water: cross-check source ↔ binário

Status: **CONFIRMADO POR SOURCE + BINÁRIO** para a arquitetura e recursos descritos aqui.

## NTerrainVertex exato

```cpp
struct NTerrainVertex
{
    uint8_t X, Y;
    uint8_t RX, RY;
};
```

Stride: **4 bytes**.

Para cada tile `(x,y)`, o cliente gera seis vértices:

```text
(0,0,x,y)
(1,0,x,y)
(1,1,x,y)
(0,0,x,y)
(1,1,x,y)
(0,1,x,y)
```

Portanto X/Y representam o canto local do tile; RX/RY representam a célula do terrain. A posição 3D é reconstruída no pipeline usando os mapas do terrain.

## Height map

O terrain cria uma texture:

```text
R32_FLOAT
TerrainSize × TerrainSize
USAGE_IMMUTABLE
BIND_SHADER_RESOURCE
```

Suporta heightmaps antigos OZB/BMP e dados 8/24-bit; no caminho 24-bit reconstrói altura de RGB e aplica offset.

## Normal map

As normais são geradas na CPU, compactadas com `packHalf4x16` e enviadas como:

```text
RGBA16_FLOAT
TerrainSize × TerrainSize
BIND_SHADER_RESOURCE
```

## Resource layout Terrain

Estáticos:

```text
VS cbCameraAttribs
VS+PS cbLightAttribs
VS g_HeightTexture
VS g_LightTexture
VS g_NormalTexture
VS g_MappingTexture
VS g_UVTexture
VS g_AttributesTexture
VS TerrainSettings
PS g_Textures
```

Mutáveis:

```text
PS g_tex2DShadowMap
PS g_tex2DFilterableShadowMap
```

## Grass

Grass é outro PSO dentro da arquitetura do Terrain, com:

```text
GrassProgram
GrassShadowProgram
GrassTextures      (2D texture array)
GrassUVTexture     (lookup/settings texture)
```

Ele reutiliza Height/Light/Normal/Mapping/Attributes/TerrainSettings e shadow map.

## Water / wind

`terrain.json` fornece:

```text
water.mod
water.mul
wind.mod
wind.mul
```

A atualização de uniform calcula:

```cpp
Settings.WaterMove = mod(WorldTime, WaterModulus) * WaterMultiplier;
Settings.WindScale = 10.0f;
Settings.WindSpeed = mod(WorldTime, WindModulus) * WindMultiplier;
```

O sistema de água desta revisão está integrado ao Terrain por parâmetros/shader; não existe evidência de um backend Vulkan separado de água.

## Implicação para nosso MU

O terrain deve ser tratado como grade compacta + textures de dados, e não como milhões de posições 3D duplicadas em VBO. Isso reduz banda e torna GL4.6/Vulkan mais consistentes.
