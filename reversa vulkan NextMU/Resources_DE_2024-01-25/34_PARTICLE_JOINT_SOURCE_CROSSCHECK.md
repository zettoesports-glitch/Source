# 34 — Particle / Joint: cross-check source ↔ binário

Status: **CONFIRMADO POR SOURCE + BINÁRIO**.

## Vertex exato

Na build não comprimida os dois usam:

```cpp
struct EffectVertex
{
    glm::vec3 Position; // 12 B
    glm::vec4 Color;    // 16 B
    glm::vec2 UV;       //  8 B
};                      // 36 B
```

A source possui tipos separados `NParticleVertex` e `NJointVertex`, mas o layout é equivalente.

Há também caminhos opcionais de build comprimida que empacotam Color/UV.

## Quads

Particle e Joint geram 4 vértices e 6 índices uint32 por elemento:

```text
0,1,2
0,2,3
```

## Particle

```text
MaxRenderCount = 50.000
VertexBuffer = sizeof(NParticleVertex) * MaxRenderCount * 4
IndexBuffer  = sizeof(uint32) * MaxRenderCount * 6
```

`NParticleSettings`:

```cpp
struct NParticleSettings
{
    float IsPremultipliedAlpha;
    float IsLinear;
}; // 8 B
```

Render groups são agrupados por `ParticleType`.

## Joint

```text
MaxRenderCount = 5000 * 50 = 250.000
VertexBuffer = sizeof(NJointVertex) * MaxRenderCount * 4
IndexBuffer  = sizeof(uint32) * MaxRenderCount * 6
```

`NJointSettings`:

```cpp
struct NJointSettings
{
    float IsPremultipliedAlpha;
}; // 4 B
```

Render groups são agrupados por `JointType`.

## Ordenação e atualização

Pending creates são ordenados por Type antes de propagação. Após criação, o registry é ordenado por `Entity::GetSort(...)`.

Há caminhos condicionais de compilação para update e render multithread através de `MUThreadsManager::Run(...)`.

## GPU buffers

Vertex/index são `USAGE_DEFAULT` e inicialmente transicionados para `VERTEX_BUFFER` / `INDEX_BUFFER`.

Settings são `USAGE_DYNAMIC + CPU_ACCESS_WRITE`.

## Arquitetura para nosso MU

Os dois sistemas podem compartilhar infraestrutura:

```text
EffectQuadStream
├─ Particle template/backend
└─ Joint template/backend
```

mantendo PSO, texture e settings específicos por tipo.
