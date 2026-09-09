# 36 — ShadowMapManager real do NextMU

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64, strings com xrefs diretos e correspondência estrutural com `DiligentFX::ShadowMapManager`.

> A source pública DiligentFX é usada somente para nomear estruturas/funções cuja sequência de operações aparece de forma equivalente no binário. O material abaixo descreve o comportamento observado no executável.

## Funções principais recuperadas

```text
0x140006520  construtor/base init do ShadowMapManager
0x140006610  Initialize / criação dos recursos de shadow
0x140006BC0  DistributeCascades / atualização de ShadowAttribs
0x1400087D0  InitializeConversionTechniques
0x1400095B0  InitializeResourceBindings
0x140009760  ConvertToFilterable
```

## InitInfo — layout observado na build

O parâmetro `r9` de `0x140006610` possui:

```text
+0x00  NumCascades            lido como uint16
+0x04  Resolution             uint32
+0x08  Format                 TEXTURE_FORMAT / uint32
+0x0C  ShadowMode             uint32
+0x10  Is32BitFilterableFmt   bool
+0x18  pComparisonSampler     pointer
+0x20  pFilterableShadowMapSampler pointer
```

O manager grava:

```text
manager +0x00 = ShadowMode
manager +0x08 = IRenderDevice*
manager +0x10 = IRenderStateCache*
```

## Shadow texture array principal

O binário cria um `TextureDesc` equivalente a:

```cpp
Name       = "Shadow map SRV";
Type       = RESOURCE_DIM_TEX_2D_ARRAY;
Width      = Resolution;
Height     = Resolution;
MipLevels  = 1;
ArraySize  = NumCascades;
Format     = Format;
BindFlags  = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL; // 0x48
```

Xref literal:

```text
"Shadow map SRV"
VA 0x1405F3570
xref 0x1400066DA
```

Criação:

```text
0x140006733 -> IRenderDevice::CreateTexture (+0x30)
```

O SRV default é obtido por:

```text
ITexture::GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE = 1)
```

em `0x140006773..0x14000677C` e armazenado em:

```text
manager +0x18 = m_pShadowMapSRV
```

Se `pComparisonSampler != nullptr`, ele é ligado ao shadow SRV.

## DSV individual por cascade

O manager redimensiona um vetor para `ArraySize` e, para cada cascade, cria uma view separada.

Literal com xref real:

```text
"Shadow map cascade DSV"
VA 0x1405F3580
xref 0x14000680B
```

`TextureViewDesc` observado:

```cpp
Name            = "Shadow map cascade DSV";
ViewType        = TEXTURE_VIEW_DEPTH_STENCIL; // 3
FirstArraySlice = cascadeIndex;
NumArraySlices  = 1;
```

Criação:

```text
0x14000685A -> ITexture::CreateView
```

Estrutura do manager:

```text
+0x20 vector begin  m_pShadowMapDSVs
+0x28 vector end
+0x30 vector capacity
```

Portanto cada cascade recebe um DSV independente sobre o mesmo texture array.

## Modos de shadow

O caminho filterable só é criado quando:

```text
ShadowMode ∈ {2, 3, 4}
```

A sequência corresponde aos modos do DiligentFX:

```text
1 = PCF
2 = VSM
3 = EVSM2
4 = EVSM4
```

O valor `1` não cria o texture array filtrável: usa diretamente o depth shadow map/PCF.

## Filterable shadow map

Para modos 2–4 o mesmo descritor muda para:

```cpp
BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET; // 0x28
```

Formato:

```text
VSM:
  32-bit -> RG32_FLOAT
  menor  -> RG16_UNORM

EVSM2:
  32-bit -> RG32_FLOAT
  menor  -> RG16_FLOAT

EVSM4:
  32-bit -> RGBA32_FLOAT
  menor  -> RGBA16_FLOAT
```

O `bool` em `InitInfo +0x10` decide a variante 32-bit.

O texture array filtrável é criado novamente por `CreateTexture` em `0x140006971`.

```text
manager +0x38 = m_pFilterableShadowMapSRV
```

## RTV por cascade filtrável

Literal com xref real:

```text
"Filterable shadow map cascade RTV"
VA 0x1405F3598
xref 0x1400069EB
```

Cada view:

```cpp
ViewType        = TEXTURE_VIEW_RENDER_TARGET; // 2
FirstArraySlice = cascadeIndex;
NumArraySlices  = 1;
```

Vetor:

```text
manager +0x40 vector begin  m_pFilterableShadowMapRTVs
manager +0x48 vector end
manager +0x50 vector capacity
```

## Intermediate texture para blur

Após criar o filterable array:

```cpp
ShadowMapDesc.ArraySize = 1;
CreateTexture(...);
```

Do texture intermediário são obtidas duas default views:

```text
manager +0x58 = Intermediate SRV  (GetDefaultView(1))
manager +0x60 = Intermediate RTV  (GetDefaultView(2))
```

Isso confirma que a filtragem separável trabalha com um alvo temporário de uma camada.

## Conversion constant buffer

`0x1400087D0` cria sob demanda:

```text
"Shadow conversion attribs CB"
size = 64 bytes
bind = BIND_UNIFORM_BUFFER
```

O buffer fica em:

```text
manager +0x68
```

Durante `ConvertToFilterable`, apenas os primeiros 24 bytes são preenchidos com a estrutura equivalente:

```cpp
struct ConversionAttribs
{
    int   Cascade;              // +0x00
    float HorizontalRadius;     // +0x04
    float VerticalRadius;       // +0x08
    float EVSMPositiveExponent; // +0x0C
    float EVSMNegativeExponent; // +0x10
    int   Is32BitEVSM;          // +0x14
};
```

O buffer é mapeado com:

```text
MAP_WRITE
MAP_FLAG_DISCARD
```

em `0x14000986D` e liberado em `0x140009963`.

## Shaders/PSOs realmente usados

Strings diretamente ligadas ao código:

```text
FullScreenTriangleVS
FullScreenTriangleVS.fx
ShadowConversions.fx

VSMHorzPS
VSM horizontal pass PS
VSM horizontal pass

EVSMHorzPS
EVSM horizontal pass PS
EVSM horizontal pass

VertBlurPS
Vertical blur pass PS
Vertical blur pass PSO

cbConversionAttribs
g_tex2DShadowMap
```

`InitializeConversionTechniques` cria um full-screen triangle VS e PSs específicos para VSM/EVSM.

PSO base:

```text
CullMode          = NONE
DepthEnable       = false
PrimitiveTopology = TRIANGLE_STRIP
NumRenderTargets  = 1
RTVFormat          = filterable shadow format
```

`cbConversionAttribs` é static resource do pixel shader.

`g_tex2DShadowMap` é resource mutável do SRB.

## SRBs

`0x1400095B0` confirma:

```text
m_ConversionTech[VSM].SRB
m_ConversionTech[EVSM2].SRB
EVSM4 reutiliza o SRB/technique compatível de EVSM2 onde aplicável
m_BlurVertTech.SRB
```

Cada conversion SRB liga:

```text
PS g_tex2DShadowMap <- manager +0x18 (depth shadow SRV)
```

O vertical blur liga:

```text
PS g_tex2DShadowMap <- manager +0x58 (intermediate SRV)
```

## ConvertToFilterable — fluxo exato

Função:

```text
0x140009760
```

Só executa para `ShadowMode 2..4`.

Para cada cascade:

```text
if FixedFilterSize == 2:
    output RTV = FilterableShadowMapRTV[cascade]
else:
    output RTV = IntermediateRTV

SetRenderTargets(... TRANSITION)
Map conversion CB
SetPipelineState(horizontal conversion PSO)
CommitShaderResources(... TRANSITION)
Draw(3 vertices, DRAW_FLAG_VERIFY_ALL)

if FixedFilterSize != 2:
    SetRenderTargets(FilterableShadowMapRTV[cascade])
    SetPipelineState(vertical blur PSO)
    CommitShaderResources(... TRANSITION)
    Draw(3 vertices)
```

Portanto:

```text
Depth cascade
    ↓
Horizontal VSM/EVSM conversion
    ↓
[optional intermediate]
    ↓
Vertical blur
    ↓
Filterable shadow cascade
```

Quando `FixedFilterSize == 2`, o código pula o blur vertical e escreve diretamente no destino final.

## Filter radius

```cpp
fixedRadius = (FixedFilterSize - 1) / 2;
```

Quando `FixedFilterSize > 0`, os raios horizontal/vertical recebem esse valor.

Quando é zero, o código deriva o raio dinamicamente de:

```text
ShadowAttribs.fFilterWorldSize
Cascade.f4LightSpaceScale.xy
shadow map Width/Height
NDC-to-UV factor 0.5
```

## Shadow map dimensions

A função de distribuição de cascades (`0x140006BC0`) lê a descrição da texture e grava em `ShadowAttribs`:

```text
f4ShadowMapDim.x = Width
f4ShadowMapDim.y = Height
f4ShadowMapDim.z = 1 / Width
f4ShadowMapDim.w = 1 / Height
```

Também diferencia OpenGL de D3D/Vulkan via informações do `IRenderDevice`, ajustando convenções de NDC/depth.

## Arquitetura para nosso MU

Esse subsistema é uma excelente referência backend-agnostic:

```text
CascadedShadowSystem
├── DepthTextureArray
│   └── DSV/attachment por cascade
├── Shadow SRV
├── opcional FilterableTextureArray
│   └── RTV/attachment por cascade
├── IntermediateTexture
├── Conversion UBO
└── Conversion PSO cache
```

Para OpenGL 4.6+:

```text
GL_TEXTURE_2D_ARRAY
FBO layer por cascade
glFramebufferTextureLayer
```

Para Vulkan:

```text
VkImage 2D array
VkImageView por layer/cascade
image layout transitions
```

A camada MU pode permanecer idêntica nos dois backends.
