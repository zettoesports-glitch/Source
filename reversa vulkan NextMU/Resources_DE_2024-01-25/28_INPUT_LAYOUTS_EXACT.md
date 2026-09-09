# 28 — InputLayouts exatos do NextMU

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 e `Diligent::LayoutElement` v2.5.4.

Registry de InputLayouts: global em `0x1409CA9D8`.
Semantic padrão: `ATTRIB` (`0x1405F36A8`).

## mesh

Registrado em torno de `0x14009DB78` / `0x14009DBD8`.

```text
ATTRIB0: float3  offset  0 stride 36  Position
ATTRIB1: float3  offset 12 stride 36  Normal
ATTRIB2: float2  offset 24 stride 36  UV
ATTRIB3: uint8x2 offset 32 stride 36  metadata/node bytes
ATTRIB4: uint16  offset 34 stride 36  source/index metadata
```

Bytes totais: `36`.

O vertex builder em `0x14003DFA0` escreve exatamente esse layout e aplica `uv.y = 1.0 - uv.y`.

## bbox

Registrado em `0x14009DDBB` / `0x14009DE1B`.

```text
ATTRIB0: float3 offset 0 stride 12
```

## terrain

Registrado em `0x14009E0EF` / `0x14009E14F` com chave `terrain` (`0x140484510`).

```text
ATTRIB0: uint8x2 offset 0 stride 4, unnormalized
ATTRIB1: uint8x2 offset 2 stride 4, unnormalized
```

Isso prova que o terrain não envia posição 3D completa por vértice. Os dados espaciais são compactos e o pipeline usa as textures `g_HeightTexture`, `g_NormalTexture`, `g_UVTexture`, etc. para reconstrução no shader.

## joint

Registrado em `0x14009E3C4`.

```text
ATTRIB0: float3 offset  0 stride 36
ATTRIB1: float4 offset 12 stride 36
ATTRIB2: float2 offset 28 stride 36
```

## particle

Registrado em `0x14009E5F5`.

Mesmo layout de Joint:

```text
ATTRIB0: float3 offset  0 stride 36
ATTRIB1: float4 offset 12 stride 36
ATTRIB2: float2 offset 28 stride 36
```

Isso confirma um vertex stream compartilhável para efeitos.

## rmlui_color

Registrado em `0x14009E826`.

```text
ATTRIB0: float2   offset  0 stride 20
ATTRIB1: uint8x4  offset  8 stride 20, normalized
ATTRIB2: float2   offset 12 stride 20
```

Estrutura equivalente:

```cpp
struct RmlVertex {
    float position[2];
    uint8_t color[4];
    float uv[2];
}; // 20 B
```

## rmlui_texture

Registrado em `0x14009E8F8` e reutiliza o mesmo layout de 20 bytes de `rmlui_color`.

## Consequência para nosso renderer

Podemos manter contratos de vertex independentes do backend:

```text
MeshVertex36
EffectVertex36
TerrainVertex4
RmlVertex20
BBoxVertex12
```

O mesmo frontend pode alimentar OpenGL 4.6+ e Vulkan.
