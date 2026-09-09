# 32 — Seleção de GPU / adapter exata

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 + `GraphicsAdapterInfo` da Diligent v2.5.4.

Função principal analisada:

```text
0x140053A40 ... 0x1400541CE
```

## Enumeração em duas etapas

O NextMU chama `IEngineFactory::EnumerateAdapters` (`vtable +0x38`) duas vezes:

1. `Adapters = nullptr` para obter `NumAdapters`;
2. aloca `NumAdapters × 0x330` bytes;
3. chama novamente para preencher `GraphicsAdapterInfo[]`.

Cada `GraphicsAdapterInfo` ocupa `0x330` bytes nessa API/build.

## Adapter ID configurado

Global observado:

```text
0x1406A7054 = Adapter ID configurado
```

`-1` significa sem ID explícito.

Quando o índice é válido, o cliente lê:

```text
adapter[index] + 0x80
```

A estrutura pública Diligent v2.5.4 confirma que `+0x80` é:

```cpp
GraphicsAdapterInfo::Type
```

O tipo é salvo em:

```text
0x1409C9468 = requested/selected adapter type
```

## Tipos de adapter

Diligent v2.5.4:

```text
0 UNKNOWN
1 SOFTWARE
2 INTEGRATED
3 DISCRETE
```

## Comportamento com preferência

Se existe um adapter configurado válido, o NextMU converte a preferência de **índice** para **tipo**.

Depois, no backend ativo, tenta encontrar um adapter do mesmo `Type`.

Isso é importante em arquitetura multi-backend porque IDs/ordem de adapters podem não ser iguais entre Vulkan, D3D11, D3D12 e OpenGL.

Se não encontra o tipo solicitado:

```text
Unable to find the requested adapter type. Using default adapter.
```

e cai na seleção automática.

## Seleção automática

A regra observada é:

### 1. Preferir maior ADAPTER_TYPE

```text
DISCRETE > INTEGRATED > SOFTWARE > UNKNOWN
```

### 2. Empate de tipo: preferir maior memória total

O assembly compara a soma dos três primeiros campos de `AdapterMemoryInfo`:

```text
LocalMemory
+ HostVisibleMemory
+ UnifiedMemory
```

Portanto, entre duas GPUs do mesmo tipo, vence a que expõe maior soma dessas memórias.

## Falhas / validação

Strings diretamente associadas:

```text
Failed to find compatible hardware adapters
Adapter ID (...) is invalid. Only ... compatible adapter(s) present in the system
Unable to find the requested adapter type. Using default adapter.
```

## Estratégia recomendada para nosso MU

Reproduzir a mesma ideia, com uma melhoria: permitir também override explícito por vendor/device quando o usuário quiser.

Default seguro:

```text
user preference
   ↓
match adapter/vendor/device quando possível
   ↓
match adapter type
   ↓
discrete > integrated > software
   ↓
maior memória útil
```

Essa política é especialmente útil em notebooks híbridos e preserva comportamento consistente entre OpenGL 4.6+ e Vulkan.
