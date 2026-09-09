# 33 — Política exata de seleção de GPU/adaptador

Status: **CONFIRMADO** no `x64/NextMU.exe` e cruzado com `GraphicsAdapterInfo` da Diligent v2.5.4.

## Função

A seleção fica em torno de:

```text
0x140053A40
```

O cliente chama `IEngineFactory::EnumerateAdapters` duas vezes:

1. para obter `NumAdapters`;
2. para preencher o array de `GraphicsAdapterInfo`.

Nesta build cada entrada é tratada com stride aproximado de `0x330` bytes.

## AdapterId explícito

A global de adapter ID é:

```text
0x1406A7054
```

Default observado:

```text
0xFFFFFFFF
```

Se o ID solicitado é válido (`AdapterId < NumAdapters`), o cliente usa esse adaptador diretamente e sincroniza o tipo desejado com o `AdapterType` da entrada escolhida.

Se é inválido, existe log equivalente a:

```text
Adapter ID (...) is invalid. Only N adapter(s) present in system
```

## AdapterType solicitado

Quando há um tipo desejado, o código procura uma entrada com esse tipo.

Se não encontra:

```text
Unable to find the requested adapter type. Using default adapter.
```

## Política default

Sem escolha explícita válida, o algoritmo seleciona primeiro o maior valor de `AdapterType`.

Na Diligent v2.5.4:

```text
UNKNOWN    = 0
SOFTWARE   = 1
INTEGRATED = 2
DISCRETE   = 3
```

Portanto a prioridade prática é:

```text
Discrete > Integrated > Software > Unknown
```

## Desempate

Quando dois adapters têm o mesmo tipo, o cliente compara:

```text
Memory.LocalMemory
+ Memory.HostVisibleMemory
+ Memory.UnifiedMemory
```

e prefere o maior total reportado.

Logo:

```text
1. maior AdapterType
2. maior memória total reportada
```

## Pseudocódigo clean-room

```cpp
int SelectAdapter(const std::vector<AdapterInfo>& adapters,
                  uint32_t requestedId,
                  AdapterType requestedType)
{
    if (requestedId < adapters.size())
        return static_cast<int>(requestedId);

    if (requestedType != AdapterType::Unknown)
    {
        for (size_t i = 0; i < adapters.size(); ++i)
            if (adapters[i].type == requestedType)
                return static_cast<int>(i);
    }

    int best = -1;
    for (size_t i = 0; i < adapters.size(); ++i)
    {
        if (best < 0 || adapters[i].type > adapters[best].type)
            best = static_cast<int>(i);
        else if (adapters[i].type == adapters[best].type &&
                 adapters[i].TotalReportedMemory() > adapters[best].TotalReportedMemory())
            best = static_cast<int>(i);
    }
    return best;
}
```

## Valor para nosso MU

Essa é uma política simples e robusta para laptops híbridos:

- respeita override do usuário;
- prefere GPU dedicada automaticamente;
- cai para integrada quando necessário;
- funciona igualmente para Vulkan e para nossa seleção/telemetria em OpenGL 4.6+.

Para nosso projeto, além disso vale manter os exports `NvOptimusEnablement` / `AmdPowerXpressRequestHighPerformance` no executável OpenGL quando aplicável, mas a seleção do device Vulkan deve continuar explícita pelo adapter enumerado.
