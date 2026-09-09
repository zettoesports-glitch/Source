# 31 — PSO cache e chave de ordenação exatos

Status: **CONFIRMADO-SOURCE / consistente com binário**.

## Cache de Pipeline State

O NextMU separa o estado em dois hashes.

### Fixed state

```text
CombinedShader
RTVFormat
DSVFormat
```

Forma `NFixedPipelineHash`.

### Dynamic state

Bitpacked em `uint64`:

```text
CullMode
EnableScissors
ColorWrite
AlphaWrite
DepthWrite
DepthFunc
StencilEnable
StencilReadMask
StencilFailOp
StencilDepthFailOp
StencilPassOp
StencilFunc
SrcBlend
DestBlend
SrcBlendAlpha
DestBlendAlpha
BlendOp
BlendOpAlpha
```

Forma `NDynamicPipelineHash`.

### Estrutura do cache

```cpp
map<NFixedPipelineHash,
    map<NDynamicPipelineHash, NPipelineState>> Pipelines;
```

`GetPipelineState()` só chama `CreateGraphicsPipelineState` quando uma combinação não existe.

## Construção do PSO

O estado dinâmico é traduzido para:

- Rasterizer/Cull;
- Blend enable, fatores, operações e write-mask;
- Depth write/function;
- Stencil;
- InputLayout do shader;
- VS/PS;
- resource layout/signatures;
- RTV/DSV formats.

Cada PSO recebe um `PipelineStateId` incremental.

`NPipelineStateInfo` mantém uma versão compacta para classificação:

```text
Shader
DepthWrite
BlendEnable
SrcBlend
DestBlend
BlendHash
```

## Shadow normalization

O caminho de shadow normaliza o estado para:

```text
CullMode       none
ColorWrite     false
AlphaWrite     false
Stencil        disabled/default
Blend          disabled
BlendOp        add
```

## Ordenação de Command Lists

`NRenderManager::Execute()` usa `std::stable_sort` por `RCommandList::Id`.

Existem dois modos:

```text
Classifier
Sequential
```

### Classifier hash

Função da snapshot:

```cpp
GetCommandListClassifiedHash(view, classify, index, shader)
```

Layout do valor retornado:

```text
63..56 view       8 bits
55..53 type       3 bits
52..51 classify   2 bits
50..43 index      8 bits
42..27 shader    16 bits
```

### Atenção: chamada real possui nomes invertidos

`PushCommandList()` chama:

```cpp
GetCommandListClassifiedHash(
    info.View,
    resolvedClassify,
    PipelineInfo->Shader,
    info.Index
);
```

Portanto, apesar dos nomes dos parâmetros da função, **na prática desta snapshot**:

```text
bits 50..43 <- PipelineInfo->Shader & 0xFF
bits 42..27 <- info.Index & 0xFFFF
```

Isso deve ser tratado como comportamento real da source de 25/01/2024 e não “corrigido” silenciosamente na documentação.

### Sequential hash

```text
63..56 view
55..53 type
31..0  sequential index
```

## Classificação de transparência

Sem blending:

```text
Opaque
```

Com blending, o blend hash é mapeado para classes alpha (`PreAlpha`/`PostAlpha`).

## Uso recomendado no nosso MU

Adotar a ideia, mas com uma chave explicitamente tipada e testável:

```text
View
Pass/Classify
Pipeline/Material domain
StableSequence
```

Evitar depender de nomes de parâmetros inconsistentes como na snapshot original.
