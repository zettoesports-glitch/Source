# 09 — Frame e lifecycle do swapchain

Status: **CONFIRMADO por assembly x64 + interface Diligent v2.5.4.**

## Frame principal recuperado

Bloco analisado:

```text
0x14002E670 ... 0x14002E7A7
```

Sequência direta:

```text
0x14002E67F GetSwapChain()
0x14002E68D ISwapChain::GetDesc()
0x14002E693 GetDeviceContext()
0x14002E6A8 ISwapChain::GetCurrentBackBufferRTV()
0x14002E6B8 ISwapChain::GetDepthBufferDSV()
0x14002E737 IDeviceContext::SetRenderTargets(...)
0x14002E751 IDeviceContext::ClearRenderTarget(...)
0x14002E776 IDeviceContext::ClearDepthStencil(...)
0x14002E7A7 ISwapChain::Present(syncInterval)
```

O parâmetro do `Present` vem de `0x1400529F0` e é convertido para `0/1`, funcionando como política equivalente a sync interval/VSync.

## Pseudocódigo clean-room equivalente

```cpp
void RenderFrame()
{
    auto* swap = GetSwapChain();
    const auto& sc = swap->GetDesc();
    auto* ctx = GetCurrentContext();

    ITextureView* rtv = swap->GetCurrentBackBufferRTV();
    ITextureView* dsv = swap->GetDepthBufferDSV();

    ctx->SetRenderTargets(1, &rtv, dsv, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    ctx->ClearRenderTarget(rtv, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    ctx->ClearDepthStencil(dsv, CLEAR_DEPTH_FLAG, 1.0f, 0,
                           RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    RenderGameSubsystems();
    swap->Present(GetSyncInterval());
}
```

Os nomes são organizacionais; a ordem das operações e interfaces é confirmada.

## Backbuffer por frame

Diligent v2.5.4 documenta que Vulkan e D3D12 retornam um `ITextureView*` diferente conforme o backbuffer muda após `Present`. O NextMU segue exatamente essa regra.

```text
ERRADO: cachear um RTV do swapchain para sempre
CERTO:  obter CurrentBackBufferRTV no frame atual
```

## Present controlado durante carregamento

Helper confirmado:

```text
0x140055290
```

Existe um contador global em `0x1409C947C`. Em caminhos de carregamento/upload o cliente acumula trabalho e executa `Present(0)` periodicamente/ao fim do lote, em vez de apresentar a cada operação.

Para nosso MU:

- não chamar Present dentro de cada upload;
- agrupar uploads;
- atualizar tela de loading em pontos controlados;
- gameplay com um Present normal por frame.

## SwapChainDesc confirmado

- `BufferCount = 2`;
- color buffer sRGB (`RGBA8_UNORM_SRGB` baseline);
- depth/stencil ajustado pelo NextMU para depth 32 + stencil (`D32_FLOAT_S8X24_UINT` observado);
- primary swapchain;
- Win32 native window entregue a `CreateSwapChainVk`.

## Shutdown

Função:

```text
0x140055110
```

Regra equivalente:

```text
parar uso GPU/contextos
-> liberar objetos do jogo
-> contexts
-> swapchain
-> device
-> factory/plugin
-> window/SDL
```

## Pendência específica

Callers exatos de `Resize`, fullscreen/windowed ainda serão promovidos a CONFIRMADO quando todos os xrefs forem nomeados. O contrato da interface já é conhecido, mas isso não será confundido com comportamento observado do jogo.
