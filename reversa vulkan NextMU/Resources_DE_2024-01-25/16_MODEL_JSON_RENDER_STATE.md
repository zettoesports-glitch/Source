# 16 — `model.json`: render-state configurável

Status: **CONFIRMADO por strings, xrefs e tabelas de inicialização estática**.

Uma das descobertas mais úteis do NextMU é que boa parte do estado de renderização de modelos não fica hardcoded. O cliente carrega `model.json` e converte strings para estados compactos usados pelo renderer.

## Chaves confirmadas

```text
model.json
model
settings
virtual_meshes
meshes
conditions
program
vertex_texture
texture
classify
mode
index
normal
lights
pre_type
pre_source
pre_value
post_type
post_source
post_value
alpha_test
premultiply_light
premultiply_alpha
hide_body
bone_head
body_height
```

## Render state confirmado

O parser em `0x14003D880..0x14003DF96` reconhece:

```text
write
  rgb
  alpha

depth
cull
depth_test
blend
  src
  dst
  equation
  equation_alpha
```

Isso permite descrever material/state sem recompilar C++.

## Comparison / depth-test table exata

A tabela inicializada em `0x1408493B0` é:

| String | Código interno |
|---|---:|
| `none` | 0 |
| `never` | 1 |
| `less` | 2 |
| `equal` | 3 |
| `less_equal` | 4 |
| `greater` | 5 |
| `not_equal` | 6 |
| `greater_equal` | 7 |
| `always` | 8 |

Os valores 1..8 correspondem à ordem do `COMPARISON_FUNCTION` da Diligent v2.5.4; `none=0` é o estado desabilitado/custom do NextMU.

## Cull table exata

Tabela em `0x1408493E0`:

| String | Código |
|---|---:|
| `none` | 1 |
| `cw` | 2 |
| `ccw` | 3 |

A orientação é armazenada pelo parser; não renomear automaticamente `cw/ccw` para FRONT/BACK sem considerar winding/front-face do PSO.

## Blend-factor table exata

Tabela em `0x1408493C0`:

| String | Código Diligent |
|---|---:|
| `zero` | 1 |
| `one` | 2 |
| `src_color` | 3 |
| `inv_src_color` | 4 |
| `src_alpha` | 5 |
| `inv_src_alpha` | 6 |
| `dst_alpha` | 7 |
| `inv_dst_alpha` | 8 |
| `dst_color` | 9 |
| `inv_dst_color` | 10 |
| `src_alpha_sat` | 11 |

Isso coincide com `BLEND_FACTOR` da Diligent v2.5.4.

## Blend-equation table exata

Tabela em `0x1408493D0`:

| String | Código |
|---|---:|
| `add` | 1 |
| `sub` | 2 |
| `revsub` | 3 |
| `min` | 4 |
| `max` | 5 |

## Outros modos encontrados no mesmo schema

```text
opaque
pre_alpha
post_alpha
light
luminosity
auto
source_set
target_set
move_speed
attack_speed
```

Esses nomes são CONFIRMADOS como parte da configuração/model pipeline, mas o significado de todos os códigos associados ainda está sendo ligado às funções consumidoras.

## Estratégia recomendada para nosso renderer

Adotar um descritor comum:

```cpp
struct MuRenderState {
    bool colorWriteRGB;
    bool colorWriteAlpha;
    bool depthWrite;
    CompareFunc depthFunc;
    CullMode cull;
    bool blendEnable;
    BlendFactor srcRGB;
    BlendFactor dstRGB;
    BlendOp opRGB;
    BlendFactor srcAlpha;
    BlendFactor dstAlpha;
    BlendOp opAlpha;
};
```

E converter esse contrato para:

```text
OpenGL 4.6 -> glDepthFunc / glBlendFuncSeparate / glBlendEquationSeparate / glCullFace...
Vulkan     -> VkPipelineDepthStencilStateCreateInfo / VkPipelineColorBlendAttachmentState / raster state
```

Assim um único `model.json` pode controlar os dois backends.

## Ganho para updates futuros

- novos materiais sem recompilar o cliente;
- correções de blend/alpha por modelo via dados;
- mesma definição visual em GL4.6 e Vulkan;
- PSO cache pode usar `MuRenderState` como parte da chave;
- facilita validar fidelidade entre os dois backends.
