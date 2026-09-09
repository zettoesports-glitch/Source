# 22 — Particle / Joint buffer layout exato

Status: **CONFIRMADO** onde indicado; semântica dos 36 bytes de cada vértice ainda está em investigação.

## Particle

Capacidade:

```text
50,000 particles
```

Buffers criados pelo `NextMU.exe`:

```text
Vertex buffer   0x6DDD00 = 7,200,000 bytes
Index buffer    0x124F80 = 1,200,000 bytes
ParticleSettings uniform = 8 bytes
```

`BufferDesc` (Diligent v2.5.4):

```text
Vertex: BIND_VERTEX_BUFFER, USAGE_DEFAULT, CPU_ACCESS_NONE
Index: BIND_INDEX_BUFFER, USAGE_DEFAULT, CPU_ACCESS_NONE
ParticleSettings: BIND_UNIFORM_BUFFER, USAGE_DYNAMIC, CPU_ACCESS_WRITE
```

Por elemento:

```text
7,200,000 / 50,000 = 144 bytes de vertex data
1,200,000 / 50,000 = 24 bytes de index data
```

A combinação fecha exatamente:

```text
4 vertices * 36 bytes = 144 bytes
6 uint32 indices      = 24 bytes
```

Portanto a geometria do particle é um **quad de quatro vértices / seis índices**.

## Joint

Capacidade:

```text
250,000 joints
```

Buffers:

```text
Vertex buffer   0x2255100 = 36,000,000 bytes
Index buffer    0x5B8D80  = 6,000,000 bytes
JointSettings uniform     = 4 bytes
```

`BufferDesc`:

```text
Vertex: BIND_VERTEX_BUFFER, USAGE_DEFAULT, CPU_ACCESS_NONE
Index: BIND_INDEX_BUFFER, USAGE_DEFAULT, CPU_ACCESS_NONE
JointSettings: BIND_UNIFORM_BUFFER, USAGE_DYNAMIC, CPU_ACCESS_WRITE
```

Por elemento:

```text
36,000,000 / 250,000 = 144 bytes
6,000,000  / 250,000 = 24 bytes
```

Novamente:

```text
4 * 36-byte vertex
6 * uint32 index
```

## Conclusão arquitetural

Particle e Joint usam a mesma topologia física de streaming. Isso permite um frontend comum:

```text
QuadStream<36-byte vertex, uint32 index>
        ├── Particle renderer
        └── Joint renderer
```

O que muda entre os dois são programa/PSO, settings e geração dos atributos, não a geometria base.

## Upload

Os grandes vertex/index buffers são `USAGE_DEFAULT`, portanto não são mapeados diretamente pela CPU como recurso persistente. O frontend acumula dados e usa o command buffer/replay para atualização/transição.

Os pequenos settings são `USAGE_DYNAMIC + CPU_ACCESS_WRITE`, adequados a `MapBuffer/UnmapBuffer`.

## Ainda em investigação

O stride de 36 bytes está provado; os nomes semânticos dos campos internos ainda precisam ser extraídos do código writer/shader. Não rotular `position/uv/color/...` até os offsets estarem diretamente ligados ao produtor/consumer.
