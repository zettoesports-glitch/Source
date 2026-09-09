# 40 — Particle/Joint RenderGroup exato

Status: **SOURCE-CORRELATED** com `mu_environment_particles.cpp`, `mu_environment_joints.cpp` e templates concretos de Particle/Joint.

## Organização CPU

Novas instâncias pendentes são ordenadas por `Type` antes de criação.
Quando entram no registry, `Entity::Info` é reordenado por `Entity::GetSort(info)`.

Antes do draw, o renderer percorre os entities e cria grupos contíguos por tipo:

```text
NRenderGroup {
    Type
    Index
    Count
}
```

Cada entity recebe:

```text
RenderGroup
RenderIndex
RenderCount
```

Se a soma ultrapassa a capacidade máxima, o entity recebe grupo inválido e não entra no batch.

## Particle

Capacidade:

```text
50,000 quads
```

## Joint

Capacidade:

```text
250,000 quads
```

## Geração de geometria

O `Template::Render()` de cada tipo preenche o shared CPU staging array no intervalo previamente reservado pelo `RenderIndex`.

Particle e Joint podem fazer essa fase em worker threads quando os respectivos macros de multithread estão habilitados.

## Upload por grupo

Um `RenderGroup()` concreto faz upload apenas da faixa que aquele grupo usa:

```text
VB offset = sizeof(Vertex) * group.Index * 4
VB size   = sizeof(Vertex) * group.Count * 4

IB offset = sizeof(uint32) * group.Index * 6
IB size   = sizeof(uint32) * group.Count * 6
```

Na primeira atualização do frame, VB/IB são transicionados:

```text
VERTEX_BUFFER -> COPY_DEST
INDEX_BUFFER  -> COPY_DEST
```

e `RequireTransition=true` evita repetir a transição para todo grupo.

## Settings uniform

Particle aloca:

```cpp
NParticleSettings {
    float IsPremultipliedAlpha;
    float IsLinear;
}
```

Joint:

```cpp
NJointSettings {
    float IsPremultipliedAlpha;
}
```

O update é enfileirado como `UpdateBufferWithMap` com:

```text
MAP_WRITE
MAP_FLAG_DISCARD
```

## PSO

Cada tipo possui seu próprio `NDynamicPipelineState`.
O fixed PSO vem do renderer compartilhado:

```text
Program = particle ou joint
RTVFormat
DSVFormat
```

Lookup:

```text
GetPipelineState(FixedPipelineState, DynamicPipelineState)
```

Static variables são ligadas uma vez por PSO:

Particle:
```text
cbCameraAttribs
ParticleSettings
```

Joint:
```text
cbCameraAttribs
JointSettings
```

## SRB

A chave principal por grupo é a texture:

```text
ResourceIds[1] = texture->GetId()
```

`ShaderResourcesBindingManager` reutiliza o SRB quando a mesma texture é usada no mesmo PSO.

Binding mutável:

```text
g_Texture
```

## DrawIndexed exato

```text
IndexCount = group.Count * 6
IndexType  = UINT32
FirstIndex = group.Index * 6
BaseVertex = group.Index * 4
Instances  = 1
```

O command order é `Classifier`, view 0.

## Exemplo de state real

`Particle Effect_V0`:

```text
Cull       = FRONT
DepthWrite = false
DepthFunc  = LESS_EQUAL
SrcBlend   = ONE
DstBlend   = ONE
Alpha src/dst = ONE/ONE
PremultipliedAlpha = false
Linear = false
```

`Joint Thunder01_V7`:

```text
Cull       = NONE
DepthWrite = false
DepthFunc  = LESS_EQUAL
SrcBlend   = ONE
DstBlend   = ONE
PremultipliedAlpha = true
```

## Consequência para nosso renderer

O batch não precisa ser uma lista de draw por particle/joint individual.
A unidade de submissão é:

```text
type/material group
  -> upload range
  -> one settings update
  -> PSO/SRB
  -> one DrawIndexed
```

Essa estratégia é diretamente transportável para GL4.6+ e Vulkan.
