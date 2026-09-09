# 6 — Status de completude da engenharia reversa

## O que está FECHADO/CONFIRMADO

### Pacote e arquitetura
- inventário completo dos 12 arquivos do ZIP;
- hashes x86/x64;
- quatro backends Diligent presentes;
- loader dinâmico por DLL/factory;
- nomes reais `_32r` / `_64r`;
- `MUGraphics::InitializeEngine` e source path do cliente;
- ordem de tentativa D3D11 -> Vulkan -> D3D12 -> OpenGL;
- constante real de ordem em `0x1404829C0` = `{1,5,2,3}`.

### Integração NextMU -> Vulkan
- endereço x64 de `MUGraphics::InitializeEngine`: `0x1400554C0`;
- case Vulkan identificado em `0x140055528`;
- loader `GraphicsEngineVk` + `GetEngineFactoryVk` confirmado por assembly;
- Diligent API version **254000** (`0x3E030`) confirmada;
- `EngineVkCreateInfo` initializer recuperado em `0x1400549B0`;
- defaults exatos dos main/dynamic descriptor pools recuperados;
- defaults exatos de memory pages/reserves, upload heap e dynamic heap recuperados;
- defaults exatos dos query pools recuperados;
- validation configurável via global do NextMU recuperada;
- mensagem ignorada `UNASSIGNED-CoreValidation-Shader-OutputNotConsumed` confirmada;
- enumeração/seleção explícita de adapter antes da criação do device confirmada;
- `CreateDeviceAndContextsVk` via vtable `+0x50` confirmada;
- `CreateSwapChainVk` via vtable `+0x58` confirmada;
- globals de factory/device/swapchain identificados;
- `SwapChainDesc` global identificado em `0x1406A7058`;
- `BufferCount = 2`;
- color sRGB `TEX_FORMAT_RGBA8_UNORM_SRGB (0x1D)`;
- depth final observado `TEX_FORMAT_D32_FLOAT_S8X24_UINT (0x14)`;
- consulta do descriptor real do swapchain após criação confirmada.

Detalhes completos em `07_EXACT_NEXTMU_INIT.md`.

### Vulkan backend
- plugin Vulkan real identificado;
- `GetEngineFactoryVk` exportado;
- `vulkan-1.dll` carregada dinamicamente;
- `vkGetInstanceProcAddr` / `vkGetDeviceProcAddr`;
- instance / physical device / logical device;
- queues e command buffers;
- Win32 surface + swapchain + acquire/present;
- buffers/textures/device memory;
- descriptor pools/layouts/sets;
- graphics/compute pipelines;
- render pass/framebuffer;
- fences/semaphores;
- dynamic heap/memory manager;
- debug utils/report + Khronos validation;
- source paths dos principais módulos Diligent Vulkan.

### Shader pipeline
- glslang presente;
- SPIR-V Tools presentes;
- HLSL -> SPIR-V confirmado;
- GLSL -> SPIR-V confirmado;
- SPIR-V reflection/resource parsing confirmado;
- HLSL2GLSL converter presente;
- grande biblioteca DiligentFX em fonte embutida no EXE;
- manifestos de shaders PBR/atmosphere/shadow identificados.

## O que está PRESENTE NO BACKEND, mas NÃO provado como usado pelo NextMU

```text
timeline semaphore
descriptor indexing
buffer device address
draw indirect count
mesh shaders
fragment shading rate
ray tracing
ray query
acceleration structures
```

Não vamos transformar capability em requisito sem xref/uso real.

## O que ainda NÃO está fechado 1:1

- source original de `mu_graphics.cpp`;
- número efetivo de frames-in-flight do renderer do jogo além do `BufferCount=2` da swapchain;
- se o NextMU sobrescreve algum tamanho de heap/pool após o initializer padrão em outros pontos;
- política exata de pipeline cache;
- lista exata de PSOs criados pelo jogo;
- bindings específicos de cada renderer MU (model/terrain/UI/effects);
- quais shaders DiligentFX estão efetivamente ativos em cada cena;
- uso real de async compute/indirect/timeline semaphore;
- lifecycle completo de resize/minimize/fullscreen/device recovery;
- revisão/commit exato do DiligentCore: o binário prova API version `254000`, mas ainda não prova um commit/tag público específico.

## Definição honesta de "completo"

A engenharia do **backend Vulkan/Diligent fornecido no pacote e da inicialização NextMU -> Vulkan está estruturalmente mapeada em profundidade suficiente para reproduzir uma integração equivalente**.

Isso não é a source original completa do NextMU. Um executável compilado não preserva todos os nomes, tipos locais, templates, comentários e organização da source.

## Próxima profundidade útil

O maior valor agora é continuar reconstruindo a camada **NextMU/MU -> renderer**, não decompilar linha por linha um backend Diligent que possui source pública:

```text
frame/present/resize
Model renderer
Terrain renderer
UI/2D
Effects
resource bindings
pipeline definitions
shader selection
frame scheduling
```

É essa camada que deve ser comparada com nosso `modernization` para construir Vulkan e OpenGL 4.6 com o mesmo contrato visual.
