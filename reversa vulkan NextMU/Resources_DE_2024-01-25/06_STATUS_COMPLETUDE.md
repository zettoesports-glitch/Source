# 6 — Status de completude da engenharia reversa

## O que está FECHADO/CONFIRMADO

### Pacote e arquitetura
- inventário completo dos 12 arquivos do ZIP;
- hashes x86/x64;
- quatro backends Diligent presentes;
- loader dinâmico por DLL/factory;
- nomes reais `_32r` / `_64r`;
- `MUGraphics::InitializeEngine` e source path do cliente;
- ordem de tentativa D3D11 -> Vulkan -> D3D12 -> OpenGL.

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
- valores exatos de todos os `EngineVkCreateInfo` usados pelo NextMU;
- número exato de frames in flight;
- tamanhos configurados de heaps/pools;
- política exata de pipeline cache;
- lista exata de PSOs criados pelo jogo;
- bindings específicos de cada renderer MU (model/terrain/UI/effects);
- quais shaders DiligentFX estão efetivamente ativos em cada cena;
- uso real de async compute/indirect/timeline semaphore;
- revisão/commit exato do DiligentCore (a build é de janeiro/2024, mas o commit ainda não foi provado pelo binário).

## Definição honesta de "completo"

A engenharia do **backend Vulkan/Diligent fornecido no pacote está estruturalmente mapeada**. Isso é suficiente para entender como integrar um backend equivalente e quais sistemas precisamos construir no nosso MU.

Não é possível chamar de source original completa do NextMU: o EXE foi compilado e parte das escolhas/configurações do jogo ainda precisa de xrefs mais profundos ou source/símbolos adicionais.

## Próxima profundidade útil

Quando formos implementar no nosso projeto, a prioridade não é decompilar cada função interna da Diligent, porque o DiligentCore é um projeto open source. O trabalho valioso é reconstruir a camada **NextMU/MU -> Diligent/Vulkan**:

```text
Model renderer
Terrain renderer
UI/2D
Effects
resource bindings
pipeline definitions
frame scheduling
```

É essa camada que deve ser comparada com nosso `modernization`.
