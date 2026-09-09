# 7 — Inicialização exata do NextMU (x64) recuperada do assembly

Este documento descreve a camada **NextMU -> Diligent/Vulkan** observada diretamente em `x64/NextMU.exe`. É a parte mais útil para reproduzir a integração no nosso cliente.

## Endereços principais

```text
MUGraphics backend bootstrap/loop : 0x140055350
MUGraphics::InitializeEngine      : 0x1400554C0
EngineVkCreateInfo initializer    : 0x1400549B0
Adapter enumeration/selection     : 0x140053A40
Context pointer vector resize     : 0x140056510
Backend DLL loader                : 0x140056390
```

Globais observados:

```text
0x1409C946C  backend/device type atual
0x1409C9480  IEngineFactory* atual
0x1409C9488  IRenderDevice*
0x1409C9490  ISwapChain*
0x1406A7050  validation/debug configuration
0x1406A7054  adapter selection/index configuration
0x1406A7058  SwapChainDesc global
```

## Ordem automática de backends

A constante em `0x1404829C0` contém literalmente:

```text
01 00 00 00
05 00 00 00
02 00 00 00
03 00 00 00
```

Logo:

```text
D3D11 -> Vulkan -> D3D12 -> OpenGL
```

O loop em `0x1400553B0` grava cada valor em `0x1409C946C`, chama `MUGraphics::InitializeEngine` e para no primeiro sucesso.

## Switch real de `MUGraphics::InitializeEngine`

Em `0x1400554F5` o backend atual é lido e despachado:

```text
1 -> D3D11
2 -> D3D12
3 -> OpenGL
4 -> mesmo caminho OpenGL
5 -> Vulkan
```

O case Vulkan começa em `0x140055528`.

## Loader Vulkan

No case Vulkan:

```text
0x140055528  "GetEngineFactoryVk"
0x14005552F  "GraphicsEngineVk"
0x140055536  call backend loader
```

O pacote real fornece:

```text
GraphicsEngineVk_64r.dll
```

O helper monta/carrega a DLL e resolve `GetEngineFactoryVk` via `GetProcAddress`.

## Diligent API version

O initializer `0x1400549B0` grava:

```text
0x0003E030 = 254000
```

no começo da estrutura base `EngineCreateInfo`.

Portanto a build foi compilada contra uma geração de API Diligent identificada pelo valor **254000**. A data/build não permite afirmar sozinho um tag/commit público exato, então isso permanece registrado como API version, não como release tag presumida.

## EngineVkCreateInfo — defaults recuperados

O assembly do initializer coincide com o layout/defaults públicos da Diligent dessa geração.

### Main descriptor pool

```text
MaxSets                8192
SeparateSampler        1024
CombinedSampler        8192
SampledImage           8192
StorageImage           1024
UniformBuffer          4096
StorageBuffer          4096
UniformTexelBuffer     1024
StorageTexelBuffer     1024
InputAttachment         256
AccelerationStructure   256
```

### Dynamic descriptor pool

```text
MaxSets                2048
SeparateSampler         256
CombinedSampler        2048
SampledImage           2048
StorageImage            256
UniformBuffer          1024
StorageBuffer          1024
UniformTexelBuffer      256
StorageTexelBuffer      256
InputAttachment          64
AccelerationStructure    64
```

### Memory / upload / dynamic heap

```text
DeviceLocalMemoryPageSize      16 MiB
HostVisibleMemoryPageSize      16 MiB
DeviceLocalMemoryReserveSize  256 MiB
HostVisibleMemoryReserveSize  256 MiB
UploadHeapPageSize               1 MiB
DynamicHeapSize                  8 MiB
DynamicHeapPageSize            256 KiB
```

### Query pools

```text
Ignored                 0
Occlusion             128
Binary occlusion      128
Timestamp              512
Pipeline statistics   128
Duration               256
```

## Validation configuration específica do NextMU

Depois de construir os defaults, o case Vulkan lê `0x1406A7050`.

Com valor >= 0, o código configura validation/debug no `EngineCreateInfo`. O nível controla `EnableValidation`/flags.

O NextMU também instala exatamente uma mensagem ignorada:

```text
UNASSIGNED-CoreValidation-Shader-OutputNotConsumed
```

O assembly grava:

```text
IgnoreDebugMessageCount = 1
ppIgnoreDebugMessageNames = &ignoredMessage
```

Isso é comportamento específico da integração do cliente, não apenas capability da Diligent.

## Enumeração e seleção de adapter

Antes da criação do device, `0x1400555E5` chama `0x140053A40`.

Essa rotina usa o método virtual da factory em `+0x38` para consultar adapters, primeiro obtendo a contagem e depois preenchendo uma lista de estruturas de `0x330` bytes cada. Ela consulta a configuração global `0x1406A7054` para escolher/validar o adapter.

Portanto o NextMU possui uma etapa explícita de **adapter enumeration/selection** antes de `CreateDeviceAndContextsVk`.

## CreateDeviceAndContextsVk

Depois da enumeração, o cliente calcula a quantidade de contextos necessária e redimensiona um vetor de ponteiros em `0x140056510`.

A chamada real acontece por vtable:

```text
IEngineFactoryVk vtable + 0x50
```

Fluxo equivalente:

```cpp
factory->CreateDeviceAndContextsVk(
    EngineCI,
    &RenderDevice,
    ContextArray
);
```

O device resultante é armazenado em:

```text
0x1409C9488
```

## CreateSwapChainVk

Se existe device, não existe swapchain ainda e há native window válida, o código chama:

```text
IEngineFactoryVk vtable + 0x58
```

com:

```text
factory
RenderDevice
ContextArray[0]
SwapChainDesc global em 0x1406A7058
NativeWindow
&SwapChain global
```

O `ISwapChain*` final fica em:

```text
0x1409C9490
```

## SwapChainDesc

O layout/defaults observados correspondem a:

```text
Width                0 (resolvido pela janela/surface)
Height               0 (resolvido pela janela/surface)
ColorBufferFormat    TEX_FORMAT_RGBA8_UNORM_SRGB = 0x1D
Usage                render target
PreTransform         optimal
BufferCount          2
DefaultDepthValue    1.0
DefaultStencilValue  0
IsPrimary            true
```

O depth inicial default da Diligent é:

```text
TEX_FORMAT_D32_FLOAT = 0x28
```

Mas durante o bootstrap, após uma consulta de compatibilidade/display, o NextMU grava o par:

```text
0x0014001D
```

Logo a configuração final observada é:

```text
Color = TEX_FORMAT_RGBA8_UNORM_SRGB    (0x1D)
Depth = TEX_FORMAT_D32_FLOAT_S8X24_UINT (0x14)
```

Isso mantém color sRGB e escolhe depth+stencil 32F/8 para a swapchain.

## Pós-criação

Após sucesso, o código consulta `ISwapChain::GetDesc()` e copia os formatos efetivamente criados para globals do cliente. Ele também marca como compatível um conjunto específico de formatos de cor observado no retorno.

Isso mostra que o cliente não assume cegamente que o driver criou exatamente o pedido: ele consulta o descriptor real do swapchain após a criação.

## Fluxo resumido fiel ao assembly

```text
Create/obtain native window
        |
        v
choose preferred color/depth formats
        |
        v
for backend in {D3D11,Vulkan,D3D12,OpenGL}
        |
        v
MUGraphics::InitializeEngine
        |
        +-> Load GraphicsEngine<API>_<arch>r.dll
        +-> resolve GetEngineFactory<API>
        +-> construct API create-info defaults
        +-> apply validation configuration
        +-> enumerate/select adapter
        +-> allocate context pointer array
        +-> CreateDeviceAndContexts...
        +-> CreateSwapChain...
        |
        v
first success wins
        |
        v
query actual SwapChainDesc
        |
        v
initialize higher-level renderer systems
```

## O que isso significa para nosso MU

Podemos reproduzir a parte boa sem copiar a source original:

1. backend loader isolado;
2. adapter enumeration/selection;
3. validation controlável por config;
4. allocator/descriptor defaults seguros;
5. 2-buffer swapchain inicialmente;
6. sRGB explícito;
7. depth/stencil explícito;
8. context array preparado antes da device creation;
9. fallback de backend;
10. consultar o descriptor real depois da criação.
