# 31 — TerrainSettings, terrain.json e integração Grass

Status: **CONFIRMADO**, exceto onde marcado como ainda sem semântica.

## TerrainSettings — buffer exato

A inicialização do terrain em `0x140051270` cria o buffer em `0x14005155F..0x1400515AC`.

`BufferDesc` observado:

```text
Size                 = 0x10 = 16 bytes
BindFlags            = 4 = BIND_UNIFORM_BUFFER
Usage                = 2 = USAGE_DYNAMIC
CPUAccessFlags       = 2 = CPU_ACCESS_WRITE
Mode                 = BUFFER_MODE_UNDEFINED
ImmediateContextMask = 1
```

O buffer é criado por `IRenderDevice::CreateBuffer` e armazenado no objeto terrain em `+0x78`.

Depois é ligado à variável estática VS:

```text
TerrainSettings
```

no PSO de terrain, e a mesma infraestrutura é usada por Grass.

## terrain.json — parâmetros confirmados

Strings:

```text
terrain.json
mod
mul
wind
scale
```

Campos CPU recuperados no objeto terrain:

```text
terrain +0xD0 = top-level mod (float)
terrain +0xD4 = top-level mul (float)
terrain +0xD8 = wind.scale (float)
terrain +0xDC = wind.mod (float)
terrain +0xE0 = wind.mul (float)
```

O objeto `wind` precisa existir no JSON para esse caminho de parsing.

## Importante sobre os 16 bytes do UBO

O objeto CPU mantém cinco floats de configuração (20 bytes), enquanto `TerrainSettings` possui 16 bytes. Portanto **não é correto assumir** que os cinco valores são copiados diretamente para o UBO.

Ainda falta mapear o writer/update do `TerrainSettings` para nomear os quatro valores efetivamente enviados ao shader.

## Terrain spatial resources

O layout de resource bindings confirmado inclui:

```text
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

O `TerrainVertex` possui apenas 4 bytes (`uint8x2 + uint8x2`), evidenciando que o terrain deixa grande parte da reconstrução espacial para shader/textures.

## Grass

O resource layout `grass` reutiliza exatamente o mesmo contrato do terrain:

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

Isso indica uma infraestrutura comum Terrain/Grass para posição, iluminação, atributos e sombras.

## Funções relevantes

```text
Terrain init       0x140051270
Terrain render     0x1400516C0
Scene call         0x140077A50
Terrain helper A   0x140052260
Terrain helper B   0x140052280
```

`0x140052260` executa uma operação de memória de `0xC0000` bytes entre campos do objeto terrain (`+0xF8/+0x100`). A semântica exata ainda não está nomeada.

## Aplicação no nosso MU

A arquitetura recomendada é manter um frontend de terrain compacto:

```text
TerrainVertex4
 + height/light/normal/attribute textures
 + TerrainSettings UBO
 + material/texture array
 + shadow resources
```

O mesmo contrato pode ser implementado por OpenGL 4.6+ e Vulkan.
