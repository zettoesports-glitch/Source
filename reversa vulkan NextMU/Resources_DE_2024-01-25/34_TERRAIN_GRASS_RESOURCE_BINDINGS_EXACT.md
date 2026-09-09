# 34 — Terrain/Grass resource bindings exatos

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 e layout de `ShaderResourceVariableDesc` da Diligent v2.5.4.

## Terrain resource layout

Construído em torno de `0x14009F94E..0x14009FC70` e registrado com chave:

```text
terrain
```

A estrutura de cada entrada tem:

```text
Name          8 B
ShaderStages  4 B
Type          1 B
Flags         1 B
padding       2 B
```

### Variáveis estáticas

```text
cbCameraAttribs       VS       STATIC
cbLightAttribs        VS|PS    STATIC
g_HeightTexture       VS       STATIC
g_LightTexture        VS       STATIC
g_NormalTexture       VS       STATIC
g_MappingTexture      VS       STATIC
g_UVTexture           VS       STATIC
g_AttributesTexture   VS       STATIC
TerrainSettings       VS       STATIC
g_Textures            PS       STATIC
```

### Variáveis mutáveis

```text
g_tex2DShadowMap            PS  MUTABLE
g_tex2DFilterableShadowMap  PS  MUTABLE
```

Isso significa que o caminho normal do terrain não precisa reconstruir descriptors para as texturas estruturais do mapa a cada draw. Somente o recurso de shadow pode variar via SRB.

## TerrainSettings

A criação do buffer está em torno de `0x14005155F..0x140051647`.

`BufferDesc` recuperado:

```text
Size            = 0x10 = 16 bytes
BindFlags       = BIND_UNIFORM_BUFFER
Usage           = USAGE_DYNAMIC
CPUAccessFlags  = CPU_ACCESS_WRITE
```

O ponteiro resultante é armazenado no objeto de recursos em `+0x78` e ligado ao static variable `TerrainSettings`.

Portanto `TerrainSettings` é um pequeno UBO/constant buffer atualizado pela CPU, não um bloco grande de terrain data.

## Texturas estruturais no objeto de terrain

No caminho de binding foram observados offsets equivalentes a:

```text
+0x28  g_HeightTexture
+0x30  g_LightTexture
+0x38  g_NormalTexture
+0x40  g_MappingTexture
+0x48  g_AttributesTexture
+0x50  g_Textures
+0x60/+0x68  variantes/recursos relacionados a UV/textures conforme caminho de qualidade
+0x78  TerrainSettings
```

A diferença `+0x60/+0x68` aparece em caminhos distintos; a semântica de cada slot permanece marcada como parcialmente reconstruída até fechar todas as variantes de qualidade.

## Grass

Logo depois do registro `terrain`, a engine cria outro resource layout e registra com chave:

```text
grass
```

O layout repete o mesmo conjunto-base de recursos de Terrain:

```text
cbCameraAttribs
cbLightAttribs
g_HeightTexture
g_LightTexture
g_NormalTexture
g_MappingTexture
g_UVTexture
g_AttributesTexture
TerrainSettings
g_Textures
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

Portanto Terrain e Grass foram desenhados para compartilhar a mesma base de dados do mapa e diferir principalmente pelo programa/PSO e pela geometria.

## Water/wind config relacionado

No carregamento de terrain há um objeto JSON `water`. Foram observadas chaves:

```text
water
mod
mul
wind
scale
```

Campos float são armazenados na estrutura de terrain em torno de `+0xD0..+0xE0`. A relação exata desses cinco valores com os quatro floats de `TerrainSettings` ainda precisa de correlação com o writer/shader e não é rotulada além do que o JSON prova.

## Valor para nosso MU

Recomendação:

```text
TerrainStaticResources
  height/light/normal/mapping/uv/attributes/textures

TerrainFrameSettings (16 B)
  pequeno buffer dinâmico

TerrainShadowResources
  mutable/per-pass
```

Isso funciona igualmente bem em OpenGL 4.6+ e Vulkan e evita rebinding desnecessário das texturas estruturais do mapa.
