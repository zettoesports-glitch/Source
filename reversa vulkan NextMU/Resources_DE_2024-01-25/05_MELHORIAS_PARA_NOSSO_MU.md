# 5 — O que vale aproveitar no nosso MU

Objetivo: construir um renderer moderno que seja **estável primeiro, rápido depois, e fácil de atualizar**.

## P0 — arquitetura que eu portaria primeiro

### 1. Backend abstraction

Separar gameplay/render logic de Vulkan/OpenGL.

```text
Game/Scene
   |
   v
Renderer API comum
   |
   +--> OpenGL 4.6
   `--> Vulkan
```

Isso reduz retrabalho futuro e permite comparar visualmente os dois backends.

### 2. Backend loader/fallback

O NextMU trata falha de API/GPU/driver como recuperável. Para nosso cliente:

```text
Vulkan
  falhou -> OpenGL 4.6
  falhou -> OpenGL 3.3/legacy
```

ou escolha manual pelo launcher/config.

### 3. Validation/debug desde o início

Em debug:

```text
VK_LAYER_KHRONOS_validation
VK_EXT_debug_utils
```

No OpenGL:

```text
KHR_debug
```

Objetivo: qualquer descriptor/layout/sync/state incorreto deve gerar log antes de virar artefato visual ou crash.

### 4. Resource lifetime centralizado

Não deixar subsistemas criarem/destruírem recursos GPU de forma ad-hoc.

Criar managers para:

```text
buffers
textures
samplers
pipelines
shaders
descriptors
frame resources
```

### 5. Dynamic ring/heap allocator

Inspirado no `VulkanDynamicHeap` da Diligent.

Usar para:

```text
FrameData
instance constants
2D vertices
particles
joint ribbons
terrain dynamic data
```

Evita alocação GPU por draw.

## P1 — desempenho de baixo/médio risco

### Descriptor/resource cache

A DLL possui:

```text
ShaderResourceCacheVk
ShaderVariableManagerVk
PipelineLayoutVk
```

Para nosso MU, manter cache de bindings por material/texture set reduz updates redundantes.

### Pipeline State Object cache

Vulkan sofre se pipeline for criado durante gameplay. Pré-criar/cachear por combinação:

```text
shader
blend
depth
cull
vertex layout
render target format
```

Mapear os estados antigos do MU para um conjunto pequeno e previsível de PSOs.

### Command batching

Combinar com o que já descobrimos no Main OpenGL 4.6:

```text
opaque model queue
2D batching
terrain command encoder
joint adjacent batching
```

No Vulkan isso é ainda mais importante porque command buffers/PSOs favorecem batches grandes e estado explícito.

### Frames-in-flight controlados

Ter uma estrutura por frame:

```text
FrameContext[0..N-1]
  command pool
  command buffer
  dynamic allocations
  descriptor allocations
  fence/semaphore
```

Começar conservador (2 ou 3), medir antes de aumentar.

## P2 — shader/visual pipeline

### Fonte de shader compartilhada

O NextMU/Diligent demonstra:

```text
HLSL/GLSL source
   -> D3D
   -> OpenGL conversion
   -> Vulkan SPIR-V
```

Para o nosso projeto, o ganho é manter o mesmo contrato visual entre OpenGL e Vulkan.

Prioridade de port:

```text
Model/BMD
Shadow
2D/UI
Terrain
Water
Sky/Atmosphere
Particles/Joints/Cloth
```

### PBR e atmosfera

O NextMU contém DiligentFX com PBR/IBL/BRDF LUT e atmosfera avançada. Isso é uma referência boa para update visual futuro, mas não deve bloquear o renderer base.

## P3 — testar só depois de estabilidade

Candidatos:

```text
timeline semaphores
descriptor indexing
buffer device address
indirect draw count
16/8-bit storage
multiview
```

Só ativar se houver ganho medido e fallback simples.

## O que eu NÃO portaria agora

```text
ray tracing
mesh shaders
fragment shading rate
acceleration structures
```

O backend suporta, mas não traz benefício proporcional para um MU clássico nesta fase.

## Roadmap combinado com nosso OpenGL 4.6

```text
Fase 1  congelar contratos (vertex/material/frame/bones)
Fase 2  estabilizar OpenGL4.6 para Hero/NPC/monster/player/bot
Fase 3  criar RenderBackend API comum
Fase 4  Vulkan bootstrap + swapchain + FrameContext
Fase 5  Model/BMD Vulkan
Fase 6  comparar GL4.6 x Vulkan frame a frame
Fase 7  2D/UI batching Vulkan
Fase 8  Terrain
Fase 9  Water/Sky/Atmosphere/Effects
Fase 10 profiling + otimizações avançadas
```

## Meta final

Não queremos apenas “ter Vulkan”. Queremos:

```text
mesma aparência
menos state bugs
menos draw overhead
menos allocations
melhor diagnóstico
fallback seguro
arquitetura fácil de atualizar
```
