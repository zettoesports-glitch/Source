# 33 — Effect Quad Stream: Particle + Joint

Status: **SOURCE-CORRELATED** com `t_graphics_layouts.h`, `t_particle_render.h` e `t_joint_render.h` históricos, consistente com buffers e InputLayouts recuperados do binário.

## Vertex exato

Particle e Joint usam o mesmo formato quando compressão está desativada nessa build:

```cpp
struct EffectVertex36
{
    glm::vec3 Position; // 12 B
    glm::vec4 Color;    // 16 B
    glm::vec2 UV;       //  8 B
};                      // 36 B
```

InputLayout:

```text
ATTRIB0 float3 Position off 0  stride 36
ATTRIB1 float4 Color    off 12 stride 36
ATTRIB2 float2 UV       off 28 stride 36
```

A source também possui caminho compilável de compressão opcional:

```text
Color -> packSnorm4x16 (uint64)
UV    -> packSnorm2x16 (uint32)
```

## Índices do quad

Cada elemento escreve 6 índices `uint32`:

```text
0,1,2
0,2,3
```

Os índices são relativos ao início do `NRenderGroup`, usando:

```text
(renderIndex - group.Index) * 4
```

## Particle

```text
MaxRenderCount = 50,000
4 vertices por particle
6 indices por particle
```

Uniform:

```cpp
struct NParticleSettings
{
    float IsPremultipliedAlpha;
    float IsLinear;
}; // 8 B
```

`NRenderBuffer` contém:

```text
Vertices[MaxRenderCount * 4]
Indices[MaxRenderCount * 6]
Groups
Program
RequireTransition
FixedPipelineState
VertexBuffer
IndexBuffer
SettingsUniform
SettingsBuffer
Bindings[PipelineStateId -> SRB]
```

A função de sprite monta billboard no view-space e suporta rotação em Z.

## Joint

```text
MaxRenderCount = 5000 * 50 = 250,000
4 vertices por joint/tail quad
6 indices por joint/tail quad
```

Uniform:

```cpp
struct NJointSettings
{
    float IsPremultipliedAlpha;
}; // 4 B
```

O `NRenderBuffer` é estruturalmente paralelo ao de Particle.

## Unificação recomendada para nosso MU

Podemos compartilhar uma infraestrutura:

```text
EffectQuadStream
├── CPU staging / packet groups
├── shared quad index pattern
├── VB/IB upload
├── PSO/SRB cache
├── resource transitions
├── Particle frontend
└── Joint frontend
```

Mantendo shaders/settings específicos por família.

## Benefício

Isso reduz duplicação entre Particle e Joint e cria base para também migrar blur ribbons/effect sprites, desde que cada tipo seja validado contra seus layouts antes de reutilizar o mesmo stream.
