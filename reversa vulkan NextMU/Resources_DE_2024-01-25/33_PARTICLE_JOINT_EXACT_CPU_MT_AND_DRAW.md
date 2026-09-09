# 33 — Particle/Joint: CPU multithread, buffers e draw exatos

Status: **CONFIRMADO-SOURCE / confirmado por tamanhos no binário**.

## Flags da build de 25/01/2024

```text
ENABLE_PARTICLE_UPDATE_MULTITHREAD = 1
ENABLE_PARTICLE_RENDER_MULTITHREAD = 1
ENABLE_JOINT_UPDATE_MULTITHREAD    = 1
ENABLE_JOINT_RENDER_MULTITHREAD    = 1

NEXTMU_COMPRESSED_PARTICLES = 0
NEXTMU_COMPRESSED_JOINTS    = 0
```

Portanto a build paraleliza **update e geração CPU da geometria**, mas usa vertex data em float, sem compressão.

## Vertex comum de Effects

Particle e Joint possuem o mesmo layout ativo:

```cpp
struct EffectVertex36
{
    float Position[3]; // 12 B
    float Color[4];    // 16 B
    float UV[2];       //  8 B
};                     // 36 B
```

InputLayout:

```text
ATTRIB0 float3 Position offset 0
ATTRIB1 float4 Color    offset 12
ATTRIB2 float2 UV       offset 28
stride 36
```

## Particle

```text
MaxRenderCount = 50000
vertices       = 50000 * 4
indices        = 50000 * 6
```

`NParticleSettings`:

```cpp
float IsPremultipliedAlpha;
float IsLinear;
```

GPU:

```text
VBO 7,200,000 B  USAGE_DEFAULT / VERTEX_BUFFER
IBO 1,200,000 B  USAGE_DEFAULT / INDEX_BUFFER
UBO         8 B  USAGE_DYNAMIC / CPU_ACCESS_WRITE
```

## Joint

```text
MaxRenderCount = 5000 * 50 = 250000
```

`NJointSettings`:

```cpp
float IsPremultipliedAlpha;
```

GPU:

```text
VBO 36,000,000 B USAGE_DEFAULT
IBO  6,000,000 B USAGE_DEFAULT
UBO          4 B USAGE_DYNAMIC / CPU_ACCESS_WRITE
```

## Quad indices

Ambos usam:

```text
0,1,2,
0,2,3
```

com base local ao grupo.

## Agrupamento

Entidades são previamente ordenadas por `Entity::GetSort(info)`.

Durante `Render()`:

1. calcula `RenderGroup`, `RenderIndex`, `RenderCount`;
2. abre um novo grupo quando `Type` muda;
3. limita ao `MaxRenderCount`;
4. `MUThreadsManager` executa geração de vertices em ranges CPU;
5. cada Template processa o seu `RenderGroup`.

## Upload por grupo

Na primeira submissão do frame, VBO/IBO passam de:

```text
VERTEX_BUFFER / INDEX_BUFFER
 -> COPY_DEST
```

Depois cada grupo atualiza **apenas seu range** com `UpdateBuffer`.

O retorno ao estado de draw é feito através da lógica de transitions agrupadas.

## Particle RenderGroup — contrato exato

Exemplo `effect_v0`:

- cull front;
- depth write false;
- depth func `LESS_EQUAL`;
- additive blending `ONE + ONE`;
- settings UBO via Map/Discard;
- static `cbCameraAttribs` e `ParticleSettings`;
- mutable `g_Texture` em SRB cache;
- bind VBO + IBO;
- `CommitShaderResources`;
- `DrawIndexed`.

Draw:

```text
NumIndices          = renderGroup.Count * 6
IndexType           = UINT32
FirstIndexLocation  = renderGroup.Index * 6
BaseVertex          = renderGroup.Index * 4
NumInstances        = 1
Flags               = VERIFY_ALL
```

## CPU/GPU architecture

```text
EnTT entities
   -> parallel CPU update
   -> parallel CPU vertex generation
   -> groups by effect type
   -> range uploads
   -> custom render command manager
   -> single immediate DeviceContext
   -> Vulkan/OpenGL backend
```

## Para nosso MU

Criar um único `EffectQuadStream` compartilhado por Particle/Joint/2D effect paths, com:

- CPU workers gerando ranges independentes;
- VBO/IBO grandes persistentes;
- atualização somente dos ranges usados;
- PSO/material grouping;
- SRB/material cache;
- upload/transition batching.
