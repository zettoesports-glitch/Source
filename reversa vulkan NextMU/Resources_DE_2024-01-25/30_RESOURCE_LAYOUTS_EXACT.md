# 30 — Resource layouts exatos (PSO/SRB)

Status: **CONFIRMADO** pelo construtor do registry em `0x14009F600...0x1400A06B6` e pela definição `ShaderResourceVariableDesc` da Diligent v2.5.4.

Registry global: `0x1409CA9E8`.

Cada registro temporário possui:

```text
name pointer (8 B)
ShaderStages (DWORD)
Type (BYTE)
Flags (BYTE)
```

Valores observados:

```text
ShaderStages 1 = VS
ShaderStages 2 = PS
ShaderStages 3 = VS|PS
Type 0 = STATIC
Type 1 = MUTABLE
Flags = 0 nos registros abaixo
```

## mesh

Registrado em `0x14009F899`, key `mesh` (`0x140639D90`).

```text
cbCameraAttribs              VS      STATIC
cbLightAttribs               VS|PS   STATIC
ModelViewProj                VS      STATIC
g_SkeletonTexture            VS      STATIC
ModelSettings                VS|PS   STATIC
g_VertexTexture              VS      MUTABLE
g_Texture                    PS      MUTABLE
g_tex2DShadowMap             PS      MUTABLE
g_tex2DFilterableShadowMap   PS      MUTABLE
```

## terrain

Registrado em `0x14009FC7E`, key `terrain` (`0x140484510`).

```text
cbCameraAttribs              VS      STATIC
cbLightAttribs               VS|PS   STATIC
g_HeightTexture              VS      STATIC
g_LightTexture               VS      STATIC
g_NormalTexture              VS      STATIC
g_MappingTexture             VS      STATIC
g_UVTexture                  VS      STATIC
g_AttributesTexture          VS      STATIC
TerrainSettings              VS      STATIC
g_Textures                   PS      STATIC
g_tex2DShadowMap             PS      MUTABLE
g_tex2DFilterableShadowMap   PS      MUTABLE
```

## grass

Registrado em `0x1400A0040`, key `grass` (`0x140485FE0`).

A build configura **o mesmo contrato de recursos do terrain**:

```text
cbCameraAttribs              VS      STATIC
cbLightAttribs               VS|PS   STATIC
g_HeightTexture              VS      STATIC
g_LightTexture               VS      STATIC
g_NormalTexture              VS      STATIC
g_MappingTexture             VS      STATIC
g_UVTexture                  VS      STATIC
g_AttributesTexture          VS      STATIC
TerrainSettings              VS      STATIC
g_Textures                   PS      STATIC
g_tex2DShadowMap             PS      MUTABLE
g_tex2DFilterableShadowMap   PS      MUTABLE
```

Isso é uma evidência forte de que grass reutiliza a infraestrutura espacial/luz/material do terrain em vez de manter um conjunto isolado de descriptors.

## joint

Registrado em `0x1400A01DB`, key `joint` (`0x140485FA8`).

```text
cbCameraAttribs   VS   STATIC
JointSettings     PS   STATIC
g_Texture         PS   MUTABLE
```

## particle

Registrado em `0x1400A035E`, key `particle` (`0x140485FB0`).

```text
cbCameraAttribs    VS   STATIC
ParticleSettings   PS   STATIC
g_Texture          PS   MUTABLE
```

## bbox

Registrado em `0x1400A0462`, key `bbox` (`0x140485FA0`).

```text
cbCameraAttribs   VS   STATIC
```

## rmlui_color

Registrado em `0x1400A056D`, key `rmlui_color` (`0x140485FC0`).

```text
cbRmlAttribs   VS|PS   STATIC
```

## rmlui_texture

Registrado em `0x1400A06AA`, key `rmlui_texture` (`0x140485FD0`).

```text
cbRmlAttribs   VS|PS   STATIC
g_Texture      PS      MUTABLE
```

## Arquitetura resultante

O NextMU faz uma separação consistente:

```text
STATIC  = câmera/luz/settings/geometry-global
MUTABLE = recursos que variam por material/draw, principalmente textures/shadow maps
```

Isso reduz descriptor churn no Vulkan e permite reproduzir o mesmo conceito em OpenGL 4.6+ com caches de binding.

## Correção de endereço

`0x1405F36A8` = `g_tex2DShadowMap`.
`ATTRIB` usado pelos InputLayouts está em `0x140485F98`.
