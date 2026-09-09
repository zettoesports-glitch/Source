# 18 — Shader resource layout: tabela recuperada

Status: **CONFIRMADO para os registros com nome carregado diretamente no builder**.

Função principal: `0x14009F600..0x1400A075B`.

O código monta registros equivalentes a `ShaderResourceVariableDesc` da Diligent. `ShaderStages` usa flags Diligent (`1=VS`, `2=PS`, `3=VS|PS`) e `Type` segue `STATIC=0`, `MUTABLE=1`.

## Model

| Name | Stages | Type |
|---|---|---|
| `cbCameraAttribs` | VS | STATIC |
| `cbLightAttribs` | VS|PS | STATIC |
| `ModelViewProj` | VS | STATIC |
| `g_SkeletonTexture` | VS | STATIC |
| `ModelSettings` | VS|PS | STATIC |
| `g_VertexTexture` | VS | MUTABLE |
| `g_Texture` | PS | MUTABLE |
| `g_tex2DShadowMap` | PS | MUTABLE |
| `g_tex2DFilterableShadowMap` | PS | MUTABLE |

## Terrain

| Name | Stages | Type |
|---|---|---|
| `g_HeightTexture` | VS | STATIC |
| `g_LightTexture` | VS | STATIC |
| `g_NormalTexture` | VS | STATIC |
| `g_MappingTexture` | VS | STATIC |
| `g_UVTexture` | VS | STATIC |
| `g_AttributesTexture` | VS | STATIC |
| `TerrainSettings` | VS | STATIC |
| `g_Textures` | PS | STATIC |
| `g_tex2DShadowMap` | PS | MUTABLE |
| `g_tex2DFilterableShadowMap` | PS | MUTABLE |

`cbCameraAttribs` e `cbLightAttribs` também são usados pelo terrain pipeline; alguns registros são construídos reutilizando registradores, portanto a tabela acima não deve ser interpretada como dump byte-a-byte de todo o array sem a segmentação final do builder.

## Joint

Confirmado no grupo `joint`:

```text
cbCameraAttribs  -> VS, STATIC
JointSettings    -> PS, STATIC
g_Texture        -> PS, MUTABLE
```

## Particle

Confirmado:

```text
ParticleSettings -> PS, STATIC
```

Outros recursos do particle pipeline podem ser reaproveitados por registrador e ainda estão sendo segmentados.

## RmlUI

Confirmado:

```text
cbRmlAttribs -> VS|PS, STATIC
```

Programas encontrados:

```text
rmlui_color
rmlui_texture
```

`g_Texture` é usado no caminho texturizado e aparece nos xrefs do renderer RmlUI.

## Nomes de grupos/programas encontrados no builder

```text
terrain
grass
joint
particle
rmlui_color
rmlui_texture
```

## Diretriz para nosso renderer

Use a mesma divisão:

- `STATIC`: frame/global/pipeline resources, ligados uma vez;
- `MUTABLE`: texturas/materials que mudam entre meshes/batches;
- `DYNAMIC`: reservar só para dados realmente atualizados por draw quando benchmark justificar.

Essa separação reduz descriptor writes no Vulkan e uniform/texture state churn no OpenGL 4.6.
