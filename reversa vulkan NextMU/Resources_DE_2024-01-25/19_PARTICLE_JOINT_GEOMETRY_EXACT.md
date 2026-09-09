# 19 — Particle + Joint: geometria e buffers exatos

Status: **CONFIRMADO por assembly x64**; interpretação de quad marcada como RECONSTRUÍDA quando indicada.

## Particle

Programa confirmado:

```text
particle
```

Capacidade:

```text
50.000 particles (0xC350)
```

Na inicialização do renderer são criados três buffers.

### Vertex buffer

```text
Size  = 7.200.000 bytes (0x6DDD00)
Bind  = BIND_VERTEX_BUFFER
Usage = USAGE_DEFAULT
```

### Index buffer

```text
Size  = 1.200.000 bytes (0x124F80)
Bind  = BIND_INDEX_BUFFER
Usage = USAGE_DEFAULT
```

### ParticleSettings

```text
Size      = 8 bytes
Bind      = BIND_UNIFORM_BUFFER
Usage     = USAGE_DYNAMIC
CPUAccess = CPU_ACCESS_WRITE
```

### Geometria por particle

```text
7.200.000 / 50.000 = 144 bytes de vertex data
1.200.000 / 50.000 = 24 bytes de index data
```

A interpretação que fecha exatamente esses números é:

```text
4 vertices × 36 bytes = 144 bytes
6 indices  ×  4 bytes =  24 bytes
```

Portanto o caminho é fortemente consistente com **quad CPU-generated + uint32 index**, 4 vértices / 6 índices por particle.

## Joint

Programa confirmado:

```text
joint
```

Capacidade:

```text
250.000 joints
```

Buffers:

### Vertex buffer

```text
36.000.000 bytes
BIND_VERTEX_BUFFER
USAGE_DEFAULT
```

### Index buffer

```text
6.000.000 bytes
BIND_INDEX_BUFFER
USAGE_DEFAULT
```

### JointSettings

```text
4 bytes
BIND_UNIFORM_BUFFER
USAGE_DYNAMIC
CPU_ACCESS_WRITE
```

Por joint:

```text
36.000.000 / 250.000 = 144 bytes
 6.000.000 / 250.000 =  24 bytes
```

O mesmo layout geométrico 4×36 + 6×uint32 se encaixa exatamente.

## Resource transitions

Depois da criação/upload dos buffers estáticos, ambos os renderers usam:

```text
IDeviceContext::TransitionResourceStates(...)
```

No final do caminho de render também existe um flush agrupado de transitions quando os buffers ficaram dirty.

## Command list de alto nível

Particle e Joint geram records de **12 bytes**:

```cpp
struct RenderCommand
{
    uint32_t RenderGroup; // +0x00
    uint32_t RenderIndex; // +0x04
    uint32_t RenderCount; // +0x08
};
```

Essa reconstrução é suportada por:

- nomes RTTI/componentes EnTT `RenderGroup`, `RenderIndex`, `RenderCount`;
- acesso do loop a +0/+4/+8;
- stride de `0x0C`;
- `RenderCount == 0` pula o record.

## Cache por RenderGroup

### Particle

```text
registry root: 0x1409CAA08
lookup:        0x1400A3220
```

### Joint

```text
registry root: 0x1409CAB68
lookup:        0x1400B5540
```

O replay mantém o `RenderGroup` anterior e só consulta o registry quando o grupo muda. Isso evita lookup/map traversal por elemento.

## Consequências para nosso MU

Vale reproduzir a ideia, não necessariamente os limites fixos:

```text
collect
→ compact command list
→ sort/group conservador
→ stream VB/IB
→ transition once
→ draw batched
```

Para GL4.6 o mesmo frontend pode alimentar VBO/IBO persistentes/streaming. Para Vulkan pode alimentar staging/dynamic upload + device-local buffers.
