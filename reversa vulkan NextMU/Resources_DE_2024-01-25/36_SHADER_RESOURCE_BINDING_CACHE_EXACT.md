# 36 — Cache de Shader Resource Binding (SRB)

Status: **CONFIRMADO-SOURCE**.

## Estrutura

Cada binding mantém:

```text
ParentId            = PipelineStateId
ShaderResourceId    = ID incremental do binding
Initialized
ShouldTransition
Resources[]         = IDs dos resources que formam a chave
Binding             = IShaderResourceBinding
```

## Chave de lookup

`GetShaderBinding()` recebe:

```text
parent/pipeline id
numResources
resource IDs
```

Os IDs são **ordenados** antes do lookup.

A busca é estruturada como:

```text
PipelineStateId
  -> number of components/resources
      -> vector de SRBs
          -> comparação exata de Resources[]
```

Logo a identidade prática é:

```text
PipelineStateId + resource-count + sorted(resource IDs)
```

## Criação

Em cache miss:

```text
parent->CreateShaderResourceBinding(..., true)
Initialized = false
ShouldTransition = true
ID incremental
```

Bindings criados durante execução paralela entram primeiro em `TemporaryShaderResourcesMap` protegido por mutex.

## Merge

`MergeTemporaryShaderBindings()`:

- move bindings temporários para o cache principal;
- agrupa por PipelineStateId e resource-count;
- ordena por `ShaderResourceId`;
- limpa o mapa temporário.

Este merge aparece nos finais de cena/render paths.

## Invalidação por resource

Há um índice reverso:

```text
ResourceId
 -> PipelineStateId
    -> ShaderResourceId
```

Quando um `NGraphicsResource` é destruído, chama:

```text
ReleaseShaderResourcesByResourceId(Id)
```

O manager remove todos os SRBs que dependem desse resource.

Isso evita bindings apontando para textures/resources destruídos.

## Transitions

Novo SRB começa com:

```text
ShouldTransition = true
```

No replay de `CommitShaderResources`:

- se `ShouldTransition`: usa `RESOURCE_STATE_TRANSITION_MODE_TRANSITION`;
- depois marca false;
- usos seguintes podem usar VERIFY em vez de repetir transitions desnecessárias.

## Por que isso é bom para Vulkan

Evita:

- recriar descriptor bindings a cada mesh;
- rebinding/retransition desnecessário;
- referências inválidas após unload de texture;
- race simples durante geração paralela de comandos.

## Versão recomendada para nosso renderer

```text
Pipeline/ResourceLayout key
+ Material/Texture IDs
-> Descriptor/SRB cache entry
```

Com:

- geração thread-safe;
- reverse dependency map;
- generation/version por resource;
- deferred destruction por frame/fence no Vulkan;
- cache hit/miss telemetry.
