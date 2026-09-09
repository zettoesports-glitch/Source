# 36 — Render order hash e replay exatos

Status: **CONFIRMADO — source histórica de 25/01/2024 + comportamento do EXE**.

## Command lists

Cada draw finaliza uma `RCommandList` contendo todos os comandos necessários para aquele draw:

```text
SetPipelineState
SetVertexBuffer
SetIndexBuffer
SetDynamicTexture/Buffer
CommitShaderResources
UpdateBuffer/Texture
Draw / DrawIndexed
```

O command list recebe um `Id` de 64 bits usado como chave de ordenação.

Antes do replay:

```cpp
std::stable_sort(CommandLists.begin(), CommandLists.begin() + Index,
    [](const RCommandList& a, const RCommandList& b) { return a.Id < b.Id; });
```

Portanto comandos com a mesma chave preservam a ordem de submissão.

## Tipos de ordem

```cpp
enum class NDrawOrderType : uint32_t
{
    Classifier,
    Sequential,
};
```

Classificações:

```cpp
enum class NRenderClassify : uint32_t
{
    None,
    Opaque,
    PreAlpha,
    PostAlpha,
};
```

## Hash Classifier

A chave de 64 bits é formada por:

```text
bits 63..56  View       (8 bits)
bits 55..53  Type       (3 bits)
bits 52..51  Classify   (2 bits)
bits 50..43  Index      (8 bits)
bits 42..27  Shader     (16 bits)
restante     livre/zero nessa função
```

Formula equivalente:

```cpp
return ((view     & 0xFF)   << 56) |
       ((type     & 0x07)   << 53) |
       ((classify & 0x03)   << 51) |
       ((index    & 0xFF)   << 43) |
       ((shader   & 0xFFFF) << 27);
```

Consequência da prioridade:

```text
View
 → ordering mode
   → render class
     → class index
       → shader
```

Isso agrupa PSOs/shaders sem quebrar as grandes classes de transparência.

## Hash Sequential

```text
bits 63..56  View
bits 55..53  Type=Sequential
bits 31..0   Index sequencial
```

Usado quando a ordem exata é mais importante do que agrupar por estado.

## Classificação automática

Se caller usa `NRenderClassify::None`, o manager calcula a classe a partir de metadata do PSO:

```text
DepthWrite
BlendEnable
SrcBlend
DestBlend
BlendHash
```

Regra principal:

```text
Blend disabled → Opaque
Blend enabled  → lookup baseado no blend hash → PreAlpha/PostAlpha
```

A build usa uma pequena LUT para mapear combinações de blend em PreAlpha/PostAlpha.

## Replay

Antes de iterar command lists, todas as transitions globais pendentes são enviadas com uma única chamada:

```cpp
TransitionResourceStates(StateTransitions.size(), StateTransitions.data());
```

Dentro de cada command list, transitions derivadas de uploads são acumuladas localmente e aplicadas imediatamente antes de `Draw`/`DrawIndexed`.

`CommitShaderResources` possui política inteligente:

```text
primeiro uso do SRB → TRANSITION
usos seguintes      → VERIFY (ou modo explícito do caller)
```

Depois do primeiro commit:

```cpp
ShaderResourceBinding->ShouldTransition = false;
```

## Por que isso é bom para Vulkan

- reduz transitions repetidas;
- agrupa shader/PSO dentro de classes seguras;
- mantém ordem estável para comandos equivalentes;
- permite ordem estrita com `Sequential` quando necessário;
- separa geração de comandos da execução gráfica.

## Aplicação no nosso MU

Podemos usar a mesma ideia, mas melhorar a chave:

```text
View
Classify
DepthBucket (somente transparência quando seguro)
PipelineKey
Material/TextureKey
Sequence
```

Para começar, a versão conservadora do NextMU é uma ótima baseline para não quebrar efeitos visuais.
