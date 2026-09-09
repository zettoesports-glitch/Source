# 32 — Particle/Joint vertex, batching e SRB exatos

Status: **CONFIRMADO — source histórica de 25/01/2024 + binário**.

## Vertex stream compartilhável

A build analisada usa as variantes não comprimidas:

```text
NEXTMU_COMPRESSED_PARTICLES = 0
NEXTMU_COMPRESSED_JOINTS    = 0
```

Formato de Particle:

```cpp
struct NParticleVertex
{
    glm::vec3 Position; // 12 B
    glm::vec4 Color;    // 16 B
    glm::vec2 UV;       //  8 B
}; // 36 B
```

Joint usa o mesmo layout físico:

```cpp
struct NJointVertex
{
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 UV;
}; // 36 B
```

InputLayout:

```text
ATTRIB0 float3 offset  0 stride 36 = Position
ATTRIB1 float4 offset 12 stride 36 = Color
ATTRIB2 float2 offset 28 stride 36 = UV
```

## Quad/index pattern

Cada efeito escreve 4 vértices e 6 índices uint32:

```text
0 1 2
0 2 3
```

Índices são locais ao início do grupo; o draw usa `BaseVertex = group.Index * 4`.

## Capacidades

Particle:

```text
MaxRenderCount  = 50,000
VertexBuffer    = 50,000 × 4 × 36 = 7,200,000 B
IndexBuffer     = 50,000 × 6 ×  4 = 1,200,000 B
ParticleSettings = 8 B
```

Joint:

```text
MaxRenderCount = 250,000
VertexBuffer   = 250,000 × 4 × 36 = 36,000,000 B
IndexBuffer    = 250,000 × 6 ×  4 =  6,000,000 B
JointSettings  = 4 B
```

## Settings exatos

```cpp
struct NParticleSettings
{
    float IsPremultipliedAlpha;
    float IsLinear;
};
```

```cpp
struct NJointSettings
{
    float IsPremultipliedAlpha;
};
```

Ambos são buffers `USAGE_DYNAMIC`, `BIND_UNIFORM_BUFFER`, `CPU_ACCESS_WRITE` e usam `MAP_WRITE | MAP_FLAG_DISCARD`.

## Organização de grupos

Particle mantém:

```cpp
struct NRenderGroup
{
    ParticleType Type;
    uint32_t Index;
    uint32_t Count;
};
```

Joint possui estrutura equivalente por `JointType`.

No Particle:

1. entidades são ordenadas por `Entity::GetSort(info)` quando necessário;
2. `Render()` percorre o registry já ordenado;
3. cria segmentos contíguos por `ParticleType`;
4. cada template do tipo preenche a faixa de vertices/indices;
5. cada grupo gera um upload da faixa usada e um draw.

## Upload e transitions

Na primeira submissão do frame/lote:

```text
VertexBuffer: VERTEX_BUFFER → COPY_DEST
IndexBuffer : INDEX_BUFFER  → COPY_DEST
```

Depois usa `UpdateBuffer` somente nas faixas do grupo, com transition mode `NONE`.

Ao final, se houve upload, restaura em lote:

```text
VertexBuffer → VERTEX_BUFFER
IndexBuffer  → INDEX_BUFFER
```

## Particle RenderGroup — pipeline real

Fluxo confirmado:

```text
UpdateBuffer(vertex range)
UpdateBuffer(index range)
Update ParticleSettings via MAP_DISCARD
GetPipelineState(fixed, dynamic)
initialize static PSO resources once
GetShaderBinding(pipelineId, resource IDs)
bind g_Texture once per cached SRB
SetPipelineState
SetVertexBuffer
SetIndexBuffer
CommitShaderResources
DrawIndexed
```

Draw:

```text
NumIndices         = group.Count * 6
IndexType          = UINT32
FirstIndexLocation = group.Index * 6
BaseVertex         = group.Index * 4
NumInstances       = 1
Flags              = DRAW_FLAG_VERIFY_ALL
```

## SRB cache exato

`NShaderResourcesBindingManager` usa chave lógica:

```text
Parent/PSO ID
+ number of resources
+ sorted list of NResourceId
```

Os Resource IDs são ordenados antes da busca, tornando a chave independente da ordem fornecida.

Estrutura:

```text
PipelineId
  └─ NumResources
      └─ vector<binding>
          └─ sorted ResourceIds[]
```

Bindings recém-criados vão para um mapa temporário protegido por mutex. `MergeTemporaryShaderBindings()` move-os para o mapa principal e ordena por `ShaderResourceId`.

Há também reverse links `ResourceId → PipelineId/binding`, permitindo invalidar SRBs quando uma texture/resource é destruída.

## Aplicação ao nosso renderer

Criar um único `EffectQuadStream` para Particle/Joint e um `ResourceBindingCache` por PSO + IDs. Isso reduz uploads, descriptors e state changes tanto no OpenGL 4.6+ quanto no Vulkan.
