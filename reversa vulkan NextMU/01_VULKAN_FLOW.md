# Fluxo Vulkan observado no NextMU

## 1. Seleção de backend

`MUGraphics::InitializeEngine` seleciona o device type e entra em um switch para D3D11, D3D12, OpenGL ou Vulkan.

No caminho Vulkan, o EXE usa o loader genérico para resolver:

```text
Library base: GraphicsEngineVk
Factory:      GetEngineFactoryVk
```

## 2. Loader dinâmico

Fluxo reconstruído:

```cpp
std::string dll = arch64
    ? "GraphicsEngineVk_64.dll"
    : "GraphicsEngineVk_32.dll";

HMODULE mod = LoadLibraryA(dll.c_str());
if (!mod)
    return nullptr;

auto getFactory = reinterpret_cast<GetFactoryFn>(
    GetProcAddress(mod, "GetEngineFactoryVk")
);

if (!getFactory)
{
    FreeLibrary(mod);
    return nullptr;
}

return getFactory;
```

Esse pseudocódigo representa o comportamento observado; nomes locais originais não sobreviveram à compilação.

## 3. Factory Vulkan

A API pública da Diligent Engine usa a seguinte arquitetura para Vulkan no Windows:

```cpp
auto* pFactoryVk = LoadAndGetEngineFactoryVk();

EngineVkCreateInfo EngineCI;
pFactoryVk->CreateDeviceAndContextsVk(
    EngineCI,
    &Device,
    &ImmediateContext
);

Win32NativeWindow Window{hWnd};
pFactoryVk->CreateSwapChainVk(
    Device,
    ImmediateContext,
    SwapChainDesc,
    Window,
    &SwapChain
);
```

Essa sequência é consistente com o caminho de factory e criação observado no NextMU.

## 4. Arquitetura relevante para o nosso MU

```text
MU game code
   |
   v
MUGraphics abstraction
   |
   +-- D3D11 factory
   +-- D3D12 factory
   +-- OpenGL factory
   `-- Vulkan factory
          |
          v
   Diligent RenderDevice
          |
          +-- DeviceContext
          +-- SwapChain
          +-- Pipeline State Objects
          +-- Buffers/Textures
          `-- Shaders
```

O benefício desse desenho é manter a lógica do jogo separada da API gráfica. Para o nosso projeto, isso é mais interessante como referência arquitetural do que copiar Diligent inteira sem necessidade.

## 5. O que ainda precisa das DLLs

Sem `GraphicsEngineVk_32.dll` / `GraphicsEngineVk_64.dll`, o EXE mostra como o backend é selecionado e carregado, mas não contém toda a implementação Vulkan de baixo nível. As DLLs são necessárias para confirmar a versão exata usada e qualquer alteração customizada do NextMU.
