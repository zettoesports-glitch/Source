# 31 — MUGraphics: cross-check source ↔ binário

Status: **CONFIRMADO POR SOURCE + BINÁRIO**.

Revisão pública usada: `b0388a6416379c03dc9089d74342180eb2560042`.

## Frontend gráfico

A camada `MUGraphics` mantém:

```text
IEngineFactory
IRenderDevice
ISwapChain
vector<IDeviceContext>
NRenderManager
SwapChainDesc
GraphicsAdapterInfo
RenderTargetDesc
```

O frontend suporta, conforme build flags:

```text
D3D11
Vulkan
D3D12
OpenGL / GLES
Metal
```

## Seleção de adapter

Se um AdapterID explícito for válido, ele é usado.

Sem AdapterID, o código seleciona por tipo, preferindo:

```text
Discrete > Integrated > Software > Unknown
```

Em empate de tipo, vence o adapter com maior soma de:

```text
LocalMemory + HostVisibleMemory + UnifiedMemory
```

## Vulkan

O caminho Vulkan usa `EngineVkCreateInfo`, obtém `IEngineFactoryVk`, escolhe adapter, cria contexts e chama:

```text
CreateDeviceAndContextsVk
CreateSwapChainVk
```

Validation pode ser habilitada por configuração. A build ignora especificamente a mensagem:

```text
UNASSIGNED-CoreValidation-Shader-OutputNotConsumed
```

## OpenGL

O caminho OpenGL usa `EngineGLCreateInfo` e `CreateDeviceAndSwapChainGL`.

A source força `NumDeferredContexts = 0` no OpenGL quando solicitado, porque esse backend não suporta deferred contexts da mesma forma.

## Swapchain

Formato de cor desejado:

```text
TEX_FORMAT_RGBA8_UNORM_SRGB
```

O frontend mantém o `SwapChainDesc` e os formatos atuais em `RenderTargetDesc`, que são usados na geração dos PSOs.

## Implicação para nosso MU

Separar:

```text
Game/Render frontend
      ↓
backend-neutral state/resources
      ↓
OpenGL 4.6+ | Vulkan
```

A seleção de GPU e a criação de device devem ficar exclusivamente no backend/bootstrap, não dentro de Model/Terrain/Effects.
