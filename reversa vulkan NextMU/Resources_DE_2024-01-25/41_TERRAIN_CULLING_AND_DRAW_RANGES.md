# 41 — Terrain culling tree e draw ranges

Status: **SOURCE-CORRELATED** com `t_terrain_cullingtree.*` e `mu_terrain.cpp` históricos.

## Packing de célula

O terrain usa grid 8-bit/256-style:

```text
PackTerrainSquare(x,y) = (y << 8) | x
```

## Árvore hierárquica

Constantes:

```text
CullingTreeSize        = 4
StartCullingTreeDepth  = 16
```

A árvore agrega blocos em grupos 4×4.

Cada `NCullingTreeBlock` mantém:

```text
SX,SY  início XY
EX,EY  fim XY
SZ     min height
EZ     max height
```

Na folha, SZ/EZ são calculados a partir dos quatro corners da célula.
Se o terreno for perfeitamente plano, o bbox recebe margem vertical ±1 para evitar degeneração.

## Construção bottom-up

As folhas representam cells individuais.
Cada nível superior agrega 4×4 filhos:

```text
SX=min
SY=min
EX=max
EY=max
SZ=min height
EZ=max height
```

## Frustum traversal

`GenerateRenderRanges()` inicia em uma grade 16×16 e chama `TraverseBlocks()`.

O AABB GPU-space usa:

```text
XY terrain grid * TerrainScale
Z = min/max height ± margem
```

Visibilidade:

```text
Invisible   -> descarta bloco inteiro
Intersecting-> desce 4×4 filhos
Visible     -> aceita toda a região sem testar cells individuais
```

Essa é a principal economia de CPU.

## Conversão para draw ranges

O resultado não é uma lista de tiles.
É:

```text
Lines = conjunto de linhas Y visíveis
Ranges[Y] = Start/End vertex
```

Como cada célula possui 6 vertices:

```text
Start = (Y * TerrainSize + SX) * 6
End   = (Y * TerrainSize + EX) * 6
```

Se múltiplos blocos visíveis caem na mesma linha, o range é expandido por min/max para formar uma faixa contínua.

## Draw final

`NTerrain::Render()` percorre:

```text
for y in RenderSettings.Lines:
    range = Ranges[y]
    Draw(
        NumVertices = range.End - range.Start,
        StartVertex = range.Start
    )
```

Terrain e Grass reutilizam os mesmos ranges visíveis.

## Resultado arquitetural

Em vez de:

```text
1 draw por tile
```

o NextMU faz aproximadamente:

```text
hierarchical frustum culling
   -> visible row spans
   -> 1 draw por range/linha
```

Isso reduz drasticamente draw calls e trabalho de culling.

## Para nosso GL4.6+/Vulkan

É uma estratégia muito adequada ao MU porque o terrain é regular e estático.

Fase inicial recomendada:

```text
compact TerrainVertex4
+ height/normal textures
+ 4x4 hierarchical culling
+ row-range draws
```

Evolução futura opcional, só após profiling:

```text
multi-draw indirect
GPU culling
compute-generated ranges
```

Não é necessário começar com compute/MDI para obter o ganho principal observado no NextMU.
