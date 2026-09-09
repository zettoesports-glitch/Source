# 14 — Diligent Engine v2.5.4 baseline exata

Status: **CONFIRMADO.**

## Identificação exata

O `NextMU.exe` grava:

```text
254000 (0x3E030)
```

na criação da engine. A tag pública `DiligentCore v2.5.4` define:

```cpp
#define DILIGENT_API_VERSION 254000
```

Portanto a baseline de API usada pelo pacote é **Diligent Engine/Core v2.5.4**.

## Consequência para a reversa

Não precisamos reconstruir manualmente cada função interna da DLL Vulkan. A source pública v2.5.4 fornece:

- ordem/layout das interfaces;
- `EngineVkCreateInfo`;
- `IEngineFactoryVk`;
- `IRenderDevice`;
- `IDeviceContext`;
- `ISwapChain`;
- implementação Vulkan base;
- shader compilation/reflection;
- resource state management.

A reversa deve focar no diferencial:

```text
NextMU configuration
NextMU resource model
NextMU PSOs
NextMU render ordering/batching
NextMU shaders/resources
NextMU frame lifecycle
```

## Factory Vulkan — offsets usados pelo EXE

```text
IEngineFactoryVk +0x50 = CreateDeviceAndContextsVk
IEngineFactoryVk +0x58 = CreateSwapChainVk
IEngineFactoryVk +0x60 = EnableDeviceSimulation
```

O assembly do NextMU usa precisamente `+0x50` e `+0x58`.

## Defaults de EngineVkCreateInfo confirmados

### MainDescriptorPoolSize

```text
MaxSets              8192
SeparateSamplers     1024
CombinedSamplers     8192
SampledImages        8192
StorageImages        1024
UniformBuffers       4096
StorageBuffers       4096
UniformTexelBuffers  1024
StorageTexelBuffers  1024
InputAttachments      256
AccelerationStructs   256
```

### DynamicDescriptorPoolSize

```text
MaxSets              2048
SeparateSamplers      256
CombinedSamplers     2048
SampledImages        2048
StorageImages         256
UniformBuffers       1024
StorageBuffers       1024
UniformTexelBuffers   256
StorageTexelBuffers   256
InputAttachments       64
AccelerationStructs    64
```

### Memory/heaps

```text
DeviceLocalMemoryPageSize     16 MB
HostVisibleMemoryPageSize     16 MB
DeviceLocalMemoryReserveSize 256 MB
HostVisibleMemoryReserveSize 256 MB
UploadHeapPageSize             1 MB
DynamicHeapSize                8 MB
DynamicHeapPageSize          256 KB
```

### Query pools

```text
Occlusion             128
Binary Occlusion      128
Timestamp             512
Pipeline Statistics   128
Duration              256
```

## Capability vs. uso

A v2.5.4 Vulkan DLL contém descriptor indexing, timeline semaphore, buffer device address, mesh/ray tracing etc. Isso descreve a **biblioteca**, não prova que o NextMU habilita cada feature.

```text
public Diligent capability != NextMU feature in use
```

Somente xref/configuração do jogo promove uma capacidade a `CONFIRMADO-USADO`.

## Estratégia para nosso projeto

Podemos usar os contratos v2.5.4 como referência sem incorporar Diligent inteira. O objetivo é frontend comum e backends OpenGL 4.6+/Vulkan estáveis.
