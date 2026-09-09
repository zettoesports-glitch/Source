# 39 — Sampler cache e lifecycle básico de textures

Status: **SOURCE-CORRELATED** com `t_graphics_samplers.cpp` e `mu_textures.cpp` históricos.

## Sampler cache

O NextMU não cria um sampler por texture. Ele compacta o estado relevante em um hash de 32 bits e reutiliza objetos `ISampler`.

Campos usados na chave:

```text
MinFilter
MagFilter
MipFilter
AddressU
AddressV
AddressW
Sampler Flags
UnnormalizedCoords
ComparisonFunc
```

A quantidade de bits de cada campo é calculada a partir dos enums Diligent (`ComputeBitsNeeded`).

Fluxo:

```text
SamplerDesc
  -> NSamplerState
  -> GetHash()
  -> Samplers.find(hash)
      hit  -> reuse NSampler
      miss -> IRenderDevice::CreateSampler
              -> GenerateResourceId()
              -> cache
```

Cada `NSampler` mantém:

```text
ResourceId
RefCntAutoPtr<ISampler>
```

Isso permite que o sampler participe da mesma arquitetura de identidade/caching de recursos usada por SRBs.

## Textures comuns do MU

Loader suporta:

```text
ozj / jpg / jpeg
ozt / tga
png
```

Wrappers MU:

```text
OZJ: pula 24 bytes antes do JPEG
OZT: pula 4 bytes antes do TGA
```

FreeImage é usado para decode.
Textures são convertidas para 32 bits e, quando necessário, BGR é trocado para RGB.

GPU texture padrão:

```text
Type      = TEX_2D
Format    = RGBA8_UNORM
Usage     = IMMUTABLE
BindFlags = SHADER_RESOURCE
```

Após criação:

```text
UNKNOWN -> SHADER_RESOURCE
```

E o sampler cacheado é ligado à default SRV.

## Filtros configuráveis

```text
linear  -> FILTER_TYPE_LINEAR
nearest -> FILTER_TYPE_POINT
```

## Address modes

```text
repeat -> TEXTURE_ADDRESS_WRAP
clamp  -> TEXTURE_ADDRESS_CLAMP
mirror -> TEXTURE_ADDRESS_MIRROR
```

## Resource identity

Cada `NGraphicsTexture` recebe um ID atômico próprio.
Esse ID é usado no `ShaderResourcesBindingManager` para localizar/reutilizar SRBs.

## Loading transaction draining

Cada texture carregada chama:

```text
IncreaseTransactions()
CheckIfRequireFlushContext()
```

Com 100 transações em backends explícitos (D3D12/Vulkan/Metal), o cliente força `swapchain->Present(0)` para drenar o trabalho de carga e limpa o contador.

Esse Present não é o frame Present normal.

## Para nosso renderer

Manter caches separados:

```text
TextureCache
SamplerCache
ResourceBindingCache
PipelineCache
```

Sampler deve ser imutável/cacheável por estado completo. Em GL4.6 isso pode mapear para sampler objects (`glGenSamplers`); em Vulkan para `VkSampler` cacheado.
