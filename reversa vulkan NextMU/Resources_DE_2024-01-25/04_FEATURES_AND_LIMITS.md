# 4 — Features presentes e limites da evidência

A DLL Vulkan é uma build completa da Diligent Engine e expõe muitas extensões. É essencial separar capacidade do backend de uso real pelo NextMU.

## Nível A — fundamentais/confirmadas para o backend Vulkan

Estas fazem parte do fluxo base recuperado:

```text
VK_KHR_surface
VK_KHR_win32_surface
VK_KHR_swapchain
vkCreateInstance
vkEnumeratePhysicalDevices
vkCreateDevice
vkCreateSwapchainKHR
vkAcquireNextImageKHR
vkQueuePresentKHR
vkCreateCommandPool
vkAllocateCommandBuffers
vkCreateDescriptorPool
vkCreateDescriptorSetLayout
vkCreateGraphicsPipelines
vkCreateSemaphore
vkCreateFence
vkAllocateMemory
vkMapMemory
```

Também estão presentes debug e validation:

```text
VK_EXT_debug_report
VK_EXT_debug_utils
VK_LAYER_KHRONOS_validation
```

## Nível B — recursos modernos presentes e úteis como candidatos

```text
VK_KHR_timeline_semaphore
VK_EXT_descriptor_indexing
VK_KHR_buffer_device_address
VK_KHR_draw_indirect_count
VK_KHR_multiview
VK_KHR_create_renderpass2
VK_KHR_maintenance1
VK_KHR_maintenance2
VK_KHR_maintenance3
VK_KHR_shader_float16_int8
VK_KHR_16bit_storage
VK_KHR_8bit_storage
VK_KHR_spirv_1_4
```

São bons candidatos para benchmark, mas não devem virar requisito do nosso renderer base sem necessidade.

## Nível C — capacidade avançada da biblioteca, NÃO comprovada como uso do jogo

```text
VK_EXT_mesh_shader
VK_KHR_fragment_shading_rate
VK_KHR_acceleration_structure
VK_KHR_ray_query
VK_KHR_ray_tracing_pipeline
VK_KHR_deferred_host_operations
```

Também existem funções como:

```text
vkCmdBuildAccelerationStructuresKHR
CreateRayTracingPipeline
BottomLevelASVkImpl
TopLevelASVkImpl
```

Isso prova que a build da Diligent suporta ray tracing/AS. Não prova que NextMU renderize nada com ray tracing.

## Limitações da análise estática atual

Ainda não é possível afirmar apenas por strings:

- qual GPU foi usada para testes do autor;
- tamanho efetivo configurado para `DynamicHeapSize` no NextMU;
- quantos descriptor pools/sets o jogo mantém por frame;
- número de frames-in-flight;
- se timeline semaphore está ativado nessa configuração;
- se o jogo usa async compute;
- se o jogo usa indirect draws;
- se o jogo usa mesh shader/ray tracing;
- política exata de pipeline cache.

Esses pontos exigem xrefs mais profundos no `NextMU.exe`, símbolos/debug adicionais ou execução instrumentada. Até lá, ficam marcados como não confirmados.

## Achado de arquitetura importante

A DLL emite alertas específicos quando o dynamic heap fica sem espaço. Isso indica que a engine foi projetada para subalocação/reuso de buffers dinâmicos, em vez de criar/destruir memória Vulkan por draw.

Para nosso MU, esse princípio é muito mais importante do que recursos sofisticados: estabilidade de allocator + descriptor reuse + PSO/resource cache primeiro.
