# 42 — Grass pass do Terrain

Status: **SOURCE-CORRELATED** com `mu_terrain.cpp`, `mu_environment_terrain.cpp` e pipeline resource histórico.

## Ativação

O pass só roda quando:

```text
GrassUVTexture != nullptr
```

Programas:

```text
normal -> GrassProgram
shadow -> GrassShadowProgram
```

Esses IDs são carregados de `terrain.json` por `grass_program`, com variante `_shadow`.

## Geometry/ranges

Grass reutiliza o mesmo `VertexBuffer` compacto do Terrain e os mesmos ranges produzidos pelo `NTerrainCullingTree`.

Ou seja:

```text
não existe um draw por tuft/grass object
```

O frontend desenha as faixas visíveis do grid; shader/mapping/UV texture definem onde e como a grass aparece.

## Dynamic state normal

```text
CullMode       = NONE
AlphaWrite     = false
DepthWrite     = false
SrcBlend       = SRC_ALPHA
DestBlend      = INV_SRC_ALPHA
SrcBlendAlpha  = SRC_ALPHA
DestBlendAlpha = INV_SRC_ALPHA
```

No shadow mode usa `DefaultShadowDynamicPipelineState`.

## Static resources

Grass reutiliza o contrato de terrain:

```text
cbCameraAttribs
g_HeightTexture
g_LightTexture
g_NormalTexture
g_MappingTexture
g_UVTexture
g_AttributesTexture
TerrainSettings
g_Textures
```

Diferenças de binding:

```text
GrassUVTexture -> g_UVTexture
GrassTextures  -> g_Textures
```

## Shadows

No main pass, se shadow está ativo, o SRB usa `ShadowResourceId` para cachear as shadow resources do mesmo jeito que Terrain/Model.

Na revisão histórica o `NEnvironment` não chama Terrain/Grass no shadow-map pass (há comentário de que grass shadows ficam visualmente ruins). Portanto a existência de `GrassShadowProgram` não significa que ele seja efetivamente desenhado no pass de cascatas dessa build.

## Draw

Depois do PSO/SRB, o pass percorre:

```text
RenderSettings.Lines
```

e para cada linha emite:

```text
Draw(End - Start, StartVertex=Start)
```

mesmo esquema do Terrain.

## Para nosso MU

Essa abordagem é muito mais escalável do que criar milhares de entidades de grama:

```text
terrain grid
+ grass mask/mapping texture
+ atlas/texture array
+ visible row ranges
+ alpha-blended pass
```

É uma boa candidata para uma futura camada visual opcional do nosso GL4.6+/Vulkan sem afetar o renderer de objetos.
