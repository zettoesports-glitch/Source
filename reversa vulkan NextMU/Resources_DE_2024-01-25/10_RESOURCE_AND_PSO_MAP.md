# 10 — Resource, PSO e binding map

Status: **misto: interfaces CONFIRMADAS; uso por subsistema confirmado onde há strings/xrefs; detalhes de cada PSO ainda em expansão.**

## Modelo de recursos do cliente

O NextMU usa a abstração Diligent em vez de Vulkan diretamente:

```text
IRenderDevice::CreateBuffer
IRenderDevice::CreateShader
IRenderDevice::CreateTexture
IRenderDevice::CreateSampler
IRenderDevice::CreateGraphicsPipelineState
IDeviceContext::SetPipelineState
IDeviceContext::CommitShaderResources
IDeviceContext::SetVertexBuffers
IDeviceContext::SetIndexBuffer
IDeviceContext::Draw / DrawIndexed
```

No Vulkan, a Diligent converte esses contratos para VkBuffer/VkImage/descriptors/pipelines e resource transitions.

## Terrain — resources recuperados

```text
cbCameraAttribs
cbLightAttribs
TerrainSettings

g_HeightTexture
g_LightTexture
g_NormalTexture
g_MappingTexture
g_UVTexture
g_AttributesTexture
g_Textures

g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

Isso revela terrain moderno baseado em múltiplas texturas de dados, não apenas textura difusa + fixed-function.

Passes/programas encontrados:

```text
terrain
terrain_shadow (_shadow)
grass_program
```

Configuração externa:

```text
terrain.json
water
wind
scale/scaled
```

## UI — RmlUI

Bindings:

```text
cbRmlAttribs
g_Texture
```

Programas:

```text
rmlui_color
rmlui_texture
```

Sampler/configuração:

```text
linear
clamp
```

`IRenderDevice::CreateBuffer` está confirmado na inicialização do renderer RmlUI.

## Resource database

O cliente possui camada de manifest:

```text
data/
resources.json
attachments
shaders
textures
models
```

A arquitetura favorece handles/objetos preparados antes do draw, em vez de cada subsystem abrir arquivos durante a renderização.

## Texture loading

Strings:

```text
linear
repeat
filter
wrap
```

Sampler state é parte explícita da descrição do recurso.

## Shader resource binding

Fluxo compatível com Diligent v2.5.4 e com a estrutura observada:

```text
CreateGraphicsPipelineState
        ↓
CreateShaderResourceBinding
        ↓
Set variables/resources
        ↓
SetPipelineState
CommitShaderResources
SetVertex/IndexBuffer
DrawIndexed
```

Para nosso Vulkan, devemos manter binding cacheado, não emitir `vkUpdateDescriptorSets` artesanalmente em todo draw.

## Otimizações úteis

- PSO cache por material/pass;
- SRB/resource-binding cache por conjunto estável;
- buffers dinâmicos via ring/suballocation;
- transitions agrupadas por subsystem;
- immutable textures/meshes criados uma vez;
- sampler cache por `filter/wrap`;
- descriptors atualizados somente quando recurso muda.

## Regra de precisão

O backend Diligent suporta compute, ray tracing, mesh shaders e indirect avançado. Até existir caller NextMU específico, isso permanece **CAPABILITY**, não feature obrigatória do renderer MU.
