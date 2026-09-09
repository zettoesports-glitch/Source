# Backend Vulkan real nas DLLs

Pacote analisado: `Resources_DE_2024-01-25.zip`.

## Binários

### x86
- `GraphicsEngineVk_32r.dll`
- PE32 / i386
- tamanho: 5,351,936 bytes
- SHA-256: `8d38d3fc0d3358ef889892782caf9cd0387085a2edaf94f95c8186a56369c41e`
- timestamp PE: 2024-01-24 23:21:38
- linker: MSVC 14.38

### x64
- `GraphicsEngineVk_64r.dll`
- PE32+ / x86-64
- tamanho: 5,923,328 bytes
- SHA-256: `1a983f85e3679710b9da7f61cbb70a916fefbb44b74dd56a77369be65e4ffba7`
- timestamp PE: 2024-01-25 01:53:10
- linker: MSVC 14.38

Ambas exportam somente:

```text
GetEngineFactoryVk
```

## Loader Vulkan

A DLL não importa `vulkan-1.dll` estaticamente. Ela possui o nome da DLL e resolve a API em runtime, começando por:

```text
vulkan-1.dll
vkGetInstanceProcAddr
vkGetDeviceProcAddr
```

Isso permite controlar melhor erro de ausência do runtime e carregar extensões de instance/device dinamicamente.

## Inicialização recuperada

Fluxo lógico do backend:

```text
GetEngineFactoryVk
  -> VulkanInstance
      -> Load vulkan-1.dll
      -> vkGetInstanceProcAddr
      -> enumerate instance extensions/layers
      -> vkCreateInstance
  -> VulkanPhysicalDevice
      -> vkEnumeratePhysicalDevices
      -> query properties/features/queues/memory
  -> VulkanLogicalDevice
      -> select queue families
      -> vkCreateDevice
      -> vkGetDeviceProcAddr
  -> RenderDeviceVk
  -> DeviceContextVk
  -> CommandQueueVk
  -> SwapChainVk
      -> Win32 surface
      -> vkCreateSwapchainKHR
      -> acquire/present
```

## Subsistemas confirmados por paths/símbolos

```text
EngineFactoryVk.cpp
VulkanInstance.cpp
VulkanPhysicalDevice.cpp
VulkanLogicalDevice.cpp
DeviceContextVkImpl.cpp
CommandQueueVkImpl.cpp
SwapChainVkImpl.cpp
FenceVkImpl.cpp
BufferVkImpl.cpp
TextureVkImpl.cpp
ShaderVkImpl.cpp
ShaderResourceCacheVk.cpp
ShaderVariableManagerVk.cpp
PipelineLayoutVk.cpp
PipelineStateVkImpl.cpp
RenderPassVkImpl.cpp
FramebufferVkImpl.cpp
DeviceMemoryVkImpl.cpp
VulkanDynamicHeap.cpp
VulkanMemoryManager.cpp
BottomLevelASVkImpl.cpp
TopLevelASVkImpl.cpp
```

## Recursos Vulkan confirmados no backend

### Apresentação
```text
VK_KHR_surface
VK_KHR_win32_surface
VK_KHR_swapchain
vkAcquireNextImageKHR
vkQueuePresentKHR
```

### Commands
```text
vkCreateCommandPool
vkAllocateCommandBuffers
```

### Resources
```text
vkAllocateMemory
vkMapMemory
vkCreateDescriptorPool
vkCreateDescriptorSetLayout
vkCreateDescriptorUpdateTemplate
```

### Pipelines
```text
vkCreateGraphicsPipelines
vkCreateComputePipelines
```

### Sync
```text
vkCreateSemaphore
vkCreateFence
```

### Debug/validation
```text
VK_EXT_debug_report
VK_EXT_debug_utils
VK_LAYER_KHRONOS_validation
```

## Features avançadas presentes na biblioteca

Existem rotinas/capacidades de ray tracing e acceleration structures dentro da Diligent/Vulkan build, por exemplo `vkBuildAccelerationStructuresKHR`. Isso prova suporte no backend, não uso pelo NextMU. Não devemos tratar essas features como requisito do nosso renderer.
