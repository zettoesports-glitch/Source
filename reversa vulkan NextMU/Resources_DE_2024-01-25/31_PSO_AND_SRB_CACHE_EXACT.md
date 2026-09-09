# 31 — Cache de PSO e SRB do NextMU

Status: **SOURCE-CORRELATED** com `t_graphics_pipelinestate.h`, `t_graphics_pipelines.cpp` e `t_graphics_shaderresources.h` do baseline histórico, consistente com o binário.

## PSO: chave em dois níveis

O cache é:

```text
FixedPipelineHash
    -> DynamicPipelineHash
        -> NPipelineState
```

Estrutura lógica:

```cpp
std::map<NFixedPipelineHash, std::map<NDynamicPipelineHash, NPipelineState>>
```

### FixedPipelineState

A chave fixa contém:

```text
CombinedShader
RTVFormat
DSVFormat
```

Ela representa aquilo que define shader + formatos dos attachments.

### DynamicPipelineState

Compactado em hash de 64 bits com:

```text
CullMode
EnableScissors
ColorWrite
AlphaWrite
DepthWrite
DepthFunc
StencilEnable
StencilReadMask
StencilFailOp
StencilDepthFailOp
StencilPassOp
StencilFunc
SrcBlend
DestBlend
SrcBlendAlpha
DestBlendAlpha
BlendOp
BlendOpAlpha
```

## Criação lazy

`GetPipelineState()`:

```text
procura fixed hash
  -> procura dynamic hash
     -> retorna existente
     -> ou CreatePipelineState()
```

Cada PSO novo recebe:

```text
PipelineIdCache++
```

Na criação, o NextMU configura:

```text
RasterizerDesc
BlendDesc
DepthStencilDesc
InputLayout
Vertex Shader
Pixel Shader
ResourceLayout/ResourceSignatures
RTVFormats
DSVFormat
```

Depois chama:

```text
IRenderDevice::CreateGraphicsPipelineState
```

## Cache de SRB

A identidade de um Shader Resource Binding é:

```text
Parent/PipelineStateId
+ número de resources
+ lista ordenada de ResourceId
```

Antes de procurar/criar:

```text
ResourceId[] é ordenado
```

Isso torna a chave independente da ordem em que os resources foram coletados.

Estrutura:

```text
PipelineStateId
   -> quantidade de resources
      -> lista de bindings
```

Há dois mapas:

```text
ShaderResourcesMap          principal
TemporaryShaderResourcesMap temporário/thread-safe
```

O temporário é mesclado posteriormente no principal e ordenado por `ShaderResourceId`.

## Reverse links

Também existe:

```text
ResourceId
   -> PipelineStateId
      -> ShaderResourceId(s)
```

Isso permite `ReleaseShaderResourcesByResourceId()` remover bindings que referenciam uma texture/resource destruída.

## Flags importantes

Cada binding mantém:

```text
Initialized
ShouldTransition
Resources[]
Binding
```

`ShouldTransition` permite que o command frontend saiba quando deve efetuar transição de resource no backend explícito.

## Por que vale portar

Essa arquitetura resolve dois problemas centrais do Vulkan:

```text
PSO explosion
Descriptor/SRB churn
```

Proposta para nosso MU:

```text
MuFixedPipelineKey
MuDynamicPipelineKey
    -> PipelineCache

PipelineId + ResourceSetKey
    -> ResourceBindingCache
```

O mesmo conceito pode ser usado em OpenGL 4.6+ para cache de program/state/texture binding, mesmo que GL não use descriptor sets nativos.
