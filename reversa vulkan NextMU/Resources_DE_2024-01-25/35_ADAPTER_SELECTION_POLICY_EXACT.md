# 35 — Política exata de seleção de GPU/adapter

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 cruzado com `DiligentCore v2.5.4`.

## Função

```text
0x140053A40
```

Ela recebe uma `IEngineFactory`, versão mínima/API e um output para `GraphicsAdapterInfo`/seleção.

## Enumeração

O código chama:

```text
IEngineFactory::EnumerateAdapters  vtable +0x38
```

em duas etapas:

1. consulta `NumAdapters` com array nulo;
2. aloca `NumAdapters * 0x330` bytes;
3. inicializa cada `GraphicsAdapterInfo`;
4. chama novamente `EnumerateAdapters` preenchendo o array.

Cada `GraphicsAdapterInfo` desta ABI ocupa `0x330` bytes.

## Tipo do adapter

O byte em `adapter +0x80` corresponde a `GraphicsAdapterInfo::Type`.

Diligent v2.5.4 define:

```text
0 = ADAPTER_TYPE_UNKNOWN
1 = ADAPTER_TYPE_SOFTWARE
2 = ADAPTER_TYPE_INTEGRATED
3 = ADAPTER_TYPE_DISCRETE
```

## Adapter ID explícito

Global:

```text
0x1406A7054 requested/selected adapter ID
```

Quando o valor é diferente de `-1`:

```cpp
if (AdapterId < NumAdapters)
{
    selected = AdapterId;
    rememberedType = Adapters[selected].Type;
}
else
{
    // log de Adapter ID inválido
    AdapterId = -1;
}
```

Strings preservadas:

```text
Adapter ID (...) is invalid. Only ... compatible adapter(s) present in the system
```

## Preservação do tipo entre backends

Global:

```text
0x1409C9468 remembered/selected AdapterType
```

Se já existe um tipo não-zero, a função procura um adapter daquele mesmo tipo na nova enumeração.

Isso é especialmente importante porque o NextMU tenta vários backends. Exemplo:

```text
D3D11 seleciona uma GPU discreta
        ↓ backend falha
Vulkan enumera adapters
        ↓
tenta manter ADAPTER_TYPE_DISCRETE
```

Se não encontrar o tipo solicitado:

```text
Unable to find the requested adapter type. Using default adapter.
```

Em seguida zera o tipo lembrado e executa a seleção default.

## Seleção default — exata

O loop default começa em adapter 0 e compara todos os adapters.

Regra primária:

```cpp
if (candidate.Type > selected.Type)
    selected = candidate;
```

Como o enum é:

```text
SOFTWARE   = 1
INTEGRATED = 2
DISCRETE   = 3
```

isso significa prioridade:

```text
DISCRETE > INTEGRATED > SOFTWARE > UNKNOWN
```

### Desempate por memória

Quando `candidate.Type == selected.Type`, o código soma três `uint64`:

```text
adapter +0x90
adapter +0x98
adapter +0xA0
```

Esses campos correspondem ao início de `GraphicsAdapterInfo::Memory` na ABI analisada e representam:

```text
LocalMemory
HostVisibleMemory
UnifiedMemory
```

A seleção fica com o adapter cuja soma é maior:

```cpp
candidateMemory = LocalMemory + HostVisibleMemory + UnifiedMemory;
selectedMemory  = ...;

if (candidateMemory > selectedMemory)
    selected = candidate;
```

## Resultado

Após selecionar, o código copia integralmente o `GraphicsAdapterInfo` de `0x330` bytes para o output e registra o adapter utilizado (`Using adapter ...`).

## Política reconstruída

```cpp
Adapter SelectAdapter(adapters, optionalId, rememberedType)
{
    if (optionalId.valid())
        return adapters[optionalId];

    if (rememberedType != UNKNOWN)
        if (auto match = firstAdapterOfType(rememberedType))
            return match;

    return max(adapters, [](a, b) {
        if (a.Type != b.Type)
            return a.Type < b.Type;

        return TotalMemory(a) < TotalMemory(b);
    });
}
```

## Relevância para nosso MU

Essa política vale copiar quase diretamente:

1. respeitar seleção explícita do usuário;
2. preservar classe de GPU ao trocar/fazer fallback de backend;
3. preferir GPU discreta por padrão;
4. em múltiplas GPUs da mesma classe, preferir maior memória reportada;
5. logar adapter/backend final de forma visível para diagnóstico.
