# 2 — Backend Vulkan real

## Binários analisados

```text
x86/GraphicsEngineVk_32r.dll
x64/GraphicsEngineVk_64r.dll
```

Export único em ambas:

```text
GetEngineFactoryVk
```

## Carregamento da API Vulkan

A DLL Vulkan não possui `vulkan-1.dll` como import estático. O runtime é carregado dinamicamente.

Strings confirmadas:

```text
vulkan-1.dll
vkGetInstanceProcAddr
vkGetDeviceProcAddr
```

Essa abordagem permite falhar com segurança quando o Vulkan runtime não existe e carregar funções por extensão/device apenas quando disponíveis.

## Fluxo reconstruído

```text
GetEngineFactoryVk
      |
      v
VulkanInstance
  - Load vulkan-1.dll
  - vkGetInstanceProcAddr
  - enumerate layers/extensions
  - vkCreateInstance
      |
      v
VulkanPhysicalDevice
  - vkEnumeratePhysicalDevices
  - properties/features
  - queue families
  - memory properties
      |
      v
VulkanLogicalDevice
  - select queues
  - vkCreateDevice
  - vkGetDeviceProcAddr
      |
      +------------------+
      |                  |
      v                  v
RenderDeviceVk       CommandQueueVk
      |                  |
      v                  v
DeviceContextVk   Command pools/buffers
      |
      v
SwapChainVk
  - Win32 surface
  - vkCreateSwapchainKHR
  - vkAcquireNextImageKHR
  - vkQueuePresentKHR
```

## Source paths preservados na DLL

O binário preserva paths do source usado no build, entre eles:

```text
DiligentCore/Graphics/GraphicsEngineVulkan/src/EngineFactoryVk.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/CommandQueueVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/DeviceContextVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/SwapChainVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/BufferVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/TextureVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/ShaderVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/ShaderResourceCacheVk.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/ShaderVariableManagerVk.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/PipelineLayoutVk.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/PipelineStateVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/RenderPassVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/FramebufferVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/FenceVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/DeviceMemoryVkImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/VulkanDynamicHeap.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanPhysicalDevice.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanLogicalDevice.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanMemoryManager.cpp
```

Também há código de acceleration structures/ray tracing no backend, mas isso é capacidade da biblioteca, não prova uso do NextMU.

## Presentation

Confirmado:

```text
VK_KHR_surface
VK_KHR_win32_surface
VK_KHR_swapchain
vkCreateSwapchainKHR
vkAcquireNextImageKHR
vkQueuePresentKHR
```

## Command submission

Confirmado:

```text
vkCreateCommandPool
vkAllocateCommandBuffers
vkCmdBindPipeline
vkCmdBindDescriptorSets
vkCmdBindVertexBuffers
vkCmdBindIndexBuffer
vkCmdDraw
vkCmdDrawIndexed
vkCmdDispatch
```

## Recursos/memória

Confirmado:

```text
vkAllocateMemory
vkMapMemory
vkCreateDescriptorPool
vkCreateDescriptorSetLayout
vkAllocateDescriptorSets
vkUpdateDescriptorSets
```

A build contém `VulkanDynamicHeap` e `VulkanMemoryManager`.

Mensagens internas mostram tratamento explícito de pressão no heap dinâmico:

```text
Increase EngineVkCreateInfo::DynamicHeapSize...
Space in dynamic heap is almost exhausted...
```

Isso é relevante para nosso MU: buffers dinâmicos de UI/particles/model constants precisam de allocator/ring buffer controlado, não alocação Vulkan por draw.

## Pipelines

Confirmado:

```text
vkCreateGraphicsPipelines
vkCreateComputePipelines
PipelineLayoutVk
PipelineStateVkImpl
```

## Sincronização

Confirmado:

```text
vkCreateSemaphore
vkCreateFence
VK_KHR_timeline_semaphore
```

A presença de timeline semaphore é capacidade do backend. O uso pelo jogo ainda precisa ser provado separadamente.

## Debug/validation

Confirmado:

```text
VK_EXT_debug_report
VK_EXT_debug_utils
VK_LAYER_KHRONOS_validation
```

Para nosso renderer, validation/debug utils devem fazer parte da build de desenvolvimento desde o início.
