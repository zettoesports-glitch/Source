# 35 — Seleção de GPU e inicialização dos backends

Status: **CONFIRMADO-SOURCE / confirmado no assembly para Vulkan**.

## Seleção de adapter

O `findAdapter` do NextMU:

1. enumera adapters compatíveis;
2. respeita `AdapterID` explícito quando válido;
3. se `AdapterType` foi pedido, procura aquele tipo;
4. no modo automático, prefere:

```text
Discrete > Integrated > Software > Unknown
```

5. em empate de tipo, escolhe o adapter com maior memória total:

```text
LocalMemory + HostVisibleMemory + UnifiedMemory
```

Isso é um mecanismo melhor que depender apenas da GPU escolhida pelo Windows.

## Vulkan

```text
GetEngineFactoryVk
EngineVkCreateInfo
validation level opcional
adapter escolhido por findAdapter
CreateDeviceAndContextsVk
CreateSwapChainVk
```

Mensagem de validation ignorada explicitamente:

```text
UNASSIGNED-CoreValidation-Shader-OutputNotConsumed
```

## D3D11

- Graphics API 11.0;
- adapter selection comum;
- enumera display modes quando adapter não é software;
- `CreateDeviceAndContextsD3D11`;
- `CreateSwapChainD3D11`.

## D3D12

- `LoadD3D12()`;
- API config 11.0 para feature requirements;
- adapter selection comum;
- display modes;
- device/contexts;
- swapchain.

## OpenGL/GLES

- `EngineGLCreateInfo.Window` recebe a janela;
- validation opcional;
- `ForceNonSeprblProgs = false` nesta snapshot;
- deferred contexts são explicitamente desativados no backend OpenGL;
- `CreateDeviceAndSwapChainGL`.

## Contexts

Depois da criação:

```text
NumImmediateContexts = max(1, EngineCI.NumImmediateContexts)
DeviceContexts = immediate + deferred contexts disponibilizados pelo backend
```

Entretanto, a análise da camada de gameplay desta build mostra execução gráfica baseada no immediate context + command manager próprio; não foi observado uso do fluxo Diligent de deferred command lists no frame principal.

## Para nosso MU

Recomendação:

- manter seleção explícita de adapter;
- preferir discrete automaticamente;
- desempatar por VRAM/memória útil;
- permitir override no config;
- usar o mesmo frontend independentemente de GL/Vulkan;
- começar Vulkan com um graphics immediate context e workers apenas para preparação CPU.
