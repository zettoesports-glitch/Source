# 32 — Terrain do NextMU: source-correlated

Status: **SOURCE-CORRELATED** com `mu_terrain.h/.cpp` históricos e evidência do binário.

## Vertex compactado

`NTerrainVertex` possui somente 4 bytes:

```cpp
struct NTerrainVertex
{
    uint8_t X;
    uint8_t Y;
    uint8_t RX;
    uint8_t RY;
};
```

Cada célula do terrain gera 6 vértices (2 triângulos):

```text
(0,0) (1,0) (1,1)
(0,0) (1,1) (0,1)
```

`RX/RY` identificam a célula no grid.

## Consequência

A posição 3D completa não precisa existir no vertex buffer. O shader recebe coordenadas compactas e consulta textures do terrain para reconstruir altura, normal, mapping e UV.

## TerrainSettings

Uniform de 16 bytes:

```cpp
struct TerrainSettings
{
    float WaterMove;
    float WindScale;
    float WindSpeed;
    float Dummy;
};
```

## Heightmap

A altura é convertida para texture:

```text
Type   = 2D
Format = R32_FLOAT
Usage  = IMMUTABLE
Bind   = SHADER_RESOURCE
```

O loader suporta altura 8-bit tradicional e também RGB de maior faixa convertido para float.

## Normal map

Normals são calculadas em CPU e empacotadas em half-float:

```text
Format = RGBA16_FLOAT
Usage  = DEFAULT
Bind   = SHADER_RESOURCE
```

Cada texel guarda a normal e um quarto componente auxiliar/zero.

## Attributes

Os atributos do chão usam `uint16` e incluem:

```text
SafeZone
Character
NoMove
NoGround
Water
Action
Height
CameraUp
NoAttackZone
```

Formato confirmado:

```text
R16_UINT
```

## Recursos do objeto Terrain

```text
HeightmapTexture
LightmapTexture
NormalTexture
MappingTexture
AttributesTexture
Textures
GrassTextures
UVTexture
GrassUVTexture
VertexBuffer
SettingsUniform
```

## Programas

```text
TerrainProgram
TerrainShadowProgram
GrassProgram
GrassShadowProgram
```

## Render state / pipeline

Terrain usa o mesmo sistema central:

```text
NFixedPipelineState
NDynamicPipelineState
GetPipelineState()
ShaderResourcesBindingManager
```

Logo Terrain/Grass não criam uma arquitetura Vulkan separada: passam pelo frontend compartilhado de PSO/SRB/command buffer.

## Oportunidade para nosso MU

Este modelo é muito eficiente para um terrain 256x256-style:

```text
4 B por vertex
height/normal/mapping em textures
shared static grid
culling por regiões
```

No nosso GL4.6+/Vulkan, vale manter o grid compacto e mover reconstrução para shader, em vez de enviar position+normal+uv completos a cada tile.
