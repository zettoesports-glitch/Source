# 16 — `model.json`: schema de render-state EXATO

Status: **SOURCE-CORRELATED / CONFIRMADO** com `mu_model.cpp` e `mu_model_mesh.h` históricos, consistente com strings/xrefs/tabelas recuperadas do EXE.

## Top-level

O loader reconhece:

```text
model                    arquivo binário do modelo
hide_body                bool
bone_head                 int16
body_height               float
settings
  virtual_meshes[]
```

O modelo carrega ainda textures, bounding boxes e dados de mesh do arquivo base.

## `virtual_meshes[]`

Cada entrada pode conter:

```text
id                       mesh index
conditions               grupos de condições
program                   shader id
vertex_texture            resource texture id
texture                   resource texture id
classify
  mode
  index
normal                    render state do modo normal
alpha                     render state do modo alpha
lights[]                  modificadores de luz
alpha_test                float
premultiply_light         bool
premultiply_alpha         bool
```

Program default da revisão:

```text
mesh_normal
```

Se `program` custom existe, o shadow program procurado é automaticamente:

```text
<program>_shadow
```

## Render state (`normal` / `alpha`)

### Write mask

```text
write
  rgb       bool
  alpha     bool
  depth     bool
```

### Cull

```text
cull: none | cw | ccw
```

Mapeamento source:

```text
none -> CULL_MODE_NONE
cw   -> CULL_MODE_FRONT
ccw  -> CULL_MODE_BACK
```

### Depth test

```text
depth_test:
  none
  never
  less
  equal
  less_equal
  greater
  not_equal
  greater_equal
  always
```

Default:

```text
LESS_EQUAL
```

### Blend

```text
blend
  src
  dst
  src_alpha
  dst_alpha
  equation
  equation_alpha
```

Factors:

```text
zero
one
src_alpha
src_color
inv_src_alpha
inv_src_color
dst_alpha
dst_color
inv_dst_alpha
inv_dst_color
src_alpha_sat
```

Equations:

```text
add
sub
revsub
min
max
```

`src`/`dst` inicializam RGB e Alpha juntos; `src_alpha`/`dst_alpha` podem sobrescrever apenas o canal alpha.
`equation` inicializa RGB e Alpha; `equation_alpha` sobrescreve somente alpha.

## Shadow state

Para cada state Normal/Alpha o cliente gera também:

```text
ShadowRenderState[Normal]
ShadowRenderState[Alpha]
```

via `NormalizeShadowRenderState(renderState)`.

Isso mantém cull/depth necessários ao material, mas normaliza propriedades incompatíveis/desnecessárias no depth-only shadow pass.

## Classificação

```text
classify.mode:
  auto
  opaque
  pre_alpha
  post_alpha

classify.index: uint32
```

Mapeamento:

```text
auto       -> None (classificação calculada pelo pipeline state)
opaque     -> Opaque
pre_alpha  -> PreAlpha
post_alpha -> PostAlpha
```

O `index` participa da order key/classifier usada pelo `RenderManager`.

## Condições de virtual mesh / lights

`conditions` é um array de grupos; cada grupo contém condições do tipo:

```json
{
  "type": "...",
  "operator": "...",
  "value": 0
}
```

Tipos confirmados:

```text
item_level
item_level_by_formula
item_rank
options_count
option_type
option_min_rank
option_max_rank
option_avg_rank
```

Operadores:

```text
equal
not_equal
less
less_equal
greater
greater_equal
```

O valor é armazenado como `uint32` nessa revisão.

Essas condições permitem variar material/virtual mesh com level/rank/options do item sem recompilar C++.

## Virtual lights

Cada light configurável usa:

```text
clamp
pre_type
pre_source
pre_value[3]
post_type
post_source
post_value[3]
conditions (opcional)
```

Tipos de operação:

```text
add
subtract
multiply
divide
inv_divide
source_set
target_set
```

Sources:

```text
none
light
luminosity
```

Estrutura correspondente possui fases Pre e Post e pode aplicar clamp [0,1].

### Nota forense sobre a baseline histórica

No commit público de 25/01/2024 existe uma atribuição suspeita no parser de `post_type/post_source`: os campos `PreType/PreSource` aparecem sendo escritos novamente onde semanticamente seriam esperados `PostType/PostSource`. Como a árvore pública é uma baseline correlacionada e não prova identidade total byte-a-byte do pacote, isso está registrado como possível bug daquela revisão, não como comportamento obrigatório do nosso renderer.

## Mesh settings resultante

```text
Program
ShadowProgram
Texture
VertexTexture
RenderState[Normal,Alpha]
ShadowRenderState[Normal,Alpha]
ClassifyMode
ClassifyIndex
AlphaTest (default 0.25)
PremultiplyLight
PremultiplyAlpha
Lights[]
```

## Estratégia para nosso renderer

Esse schema deve inspirar um contrato backend-agnostic:

```text
Model material JSON
 -> MuMaterialState
 -> classification/order key
 -> PSO cache
 -> SRB/resource key
 -> GL4.6 or Vulkan backend
```

Ganho: podemos corrigir blend, alpha, cull, depth, shader e virtual variants por dados, mantendo GL4.6+ e Vulkan visualmente alinhados.
