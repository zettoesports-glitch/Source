# 37 — Cascaded Shadow Pipeline do NextMU

Status: **SOURCE-CORRELATED** com `mu_environment.cpp`, `mu_renderstate.*` e `DiligentFX ShadowMapManager`, consistente com recursos/macros encontrados no EXE.

## Implementação base

O NextMU usa:

```text
Diligent::ShadowMapManager
```

O objeto pertence ao `NEnvironment` e recebe um `ShadowResourceId` próprio para participar do cache de SRB.

## Init

Só é criado quando shadows estão habilitados.

Configuração:

```text
Format       = ShadowMapDepthFormat
Resolution   = config
NumCascades  = config
ShadowMode   = config
```

Samplers:

### Comparison / PCF
```text
ComparisonFunc = LESS
Min/Mag/Mip = COMPARISON_LINEAR
```

### Filterable shadow
```text
Min/Mag/Mip = ANISOTROPIC
MaxAnisotropy = LightAttribs.ShadowAttribs.iMaxAnisotropy
```

## Cascade distribution

Por update:

```text
camera view
shadow projection
light direction
partitioning factor
```

são fornecidos ao `ShadowMapManager::DistributeCascades()`.

O range global de Z é ajustado em escala exponencial (~base 1.1) e `MinZ` é limitado a pelo menos 10.

Para cada cascade o cliente extrai um frustum próprio para culling.

## Bias por resolução

```text
>= 2048 -> 0.0025
>= 1024 -> 0.0050
<  1024 -> 0.0075
```

## Shadow render pass

Para cada cascade:

```text
build shadow CameraAttribs
SetRenderTargets(depth-only)
ClearDepthStencil
Objects->Render
Characters->Render
RenderManager->Execute
reset model/bbox arenas
```

Na revisão analisada:

```text
Terrain shadow draw está comentado/desabilitado
```

com nota de que grass shadows ficam ruins pelo método usado.

## Estado final Vulkan

Depois das cascades, a depth texture é transicionada para:

```text
Vulkan -> RESOURCE_STATE_DEPTH_READ
outros -> RESOURCE_STATE_SHADER_RESOURCE
```

Essa diferença de backend é explícita na source.

## Modos filterable

Quando o modo não é PCF:

```text
ShadowMap->ConvertToFilterable()
```

e o resultado é transicionado de `RENDER_TARGET` para `SHADER_RESOURCE`.

## Main pass

No render normal o cliente disponibiliza ao Model/Terrain:

```text
ShadowMap
ShadowResourceId
ShadowMode
```

Os shaders escolhem entre:

```text
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

## Macros de compilação associadas

```text
USE_SHADOW
SHADOW_MODE
SHADOW_FILTER_SIZE
FILTER_ACROSS_CASCADES
BEST_CASCADE_SEARCH
```

## Para nosso MU

A parte que vale copiar é a arquitetura:

```text
shadow pass depth-only em cascatas
-> culling por cascade
-> recurso global cacheado
-> main pass lê shadow SRV
```

Não precisamos obrigatoriamente usar DiligentFX; podemos implementar o mesmo contrato no nosso frontend e ter implementação GL4.6+/Vulkan específica.
