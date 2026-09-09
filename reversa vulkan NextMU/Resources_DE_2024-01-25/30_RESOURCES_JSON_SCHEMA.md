# 30 — `resources.json`: schema e pipeline de assets

Status: **CONFIRMADO** onde indicado por assembly/string xrefs do `NextMU.exe` x64. Campos ainda não totalmente decodificados são explicitamente marcados.

Arquivo carregado: `data/resources.json`. Loader principal: `0x140083050`.

## Objeto raiz

O parser procura quatro coleções principais:

```json
{
  "attachments": { ... },
  "shaders": { ... },
  "textures": { ... },
  "models": { ... }
}
```

Strings/VAs: `attachments=0x140484960`, `shaders=0x140484970`, `textures=0x140481D58`; `models` usa string compartilhada do resource/model system.

## `attachments`

O loader itera os nomes da coleção e chama o registrador em `0x14008CD30`. O registrador mantém um mapa global próximo de `0x1409CA608`, aloca um registro de aproximadamente `0x48` bytes e associa um ID de 16 bits. `0xFFFF` aparece como sentinel de falha/não encontrado.

A evidência atual prova o **registro por nome/ID**; não atribuímos semântica não comprovada aos valores JSON dessa coleção.

## `shaders`

Parser por shader: `0x140086250`.

Campos confirmados:

```json
{
  "id": "...",
  "vertex": "...",
  "fragment": "...",
  "resource_id": "...",
  "macros": { ... }
}
```

`fragment` é construído inline no código; os demais nomes aparecem diretamente nas strings do PE. O parser valida tipos JSON e falha em entradas malformadas.

### Macros do shader

Cada macro do arquivo pode possuir:

```json
{
  "type": "integer | float | boolean | string",
  "value": ...
}
```

Os valores são convertidos para texto antes de formar o array de `Diligent::ShaderMacro`.

### Macros internas acrescentadas pelo cliente

O helper `0x140084530` acrescenta:

```text
SKELETON_TEXTURE_WIDTH  = 2048
SKELETON_TEXTURE_HEIGHT = 512
USE_SHADOW
SHADOW_MODE
SHADOW_FILTER_SIZE
FILTER_ACROSS_CASCADES
BEST_CASCADE_SEARCH
```

Os cinco últimos recebem valores da configuração/runtime de shadows.

### Compatibilidade cross-backend

O helper consulta o RenderDeviceType por `0x1400552E0` e chama `0x140082C70`. Para OpenGL/OpenGLES, esse helper acrescenta aliases de tipos entre:

```text
half   <-> float
half2  <-> float2
half3  <-> float3
half4  <-> float4
```

Isso é uma camada de compatibilidade de shader, não um parser de attachments.

### `ShaderCreateInfo` exato

A criação usa `IRenderDevice +0x28 = CreateShader`.

Primeira criação em `0x140085BBA`:

```text
Desc.ShaderType = 1 = SHADER_TYPE_VERTEX
Desc.UseCombinedTextureSamplers = true
SourceLanguage = 1 = SHADER_SOURCE_LANGUAGE_HLSL
```

Se o VS foi criado, a segunda criação em `0x140085DCE` muda:

```text
Desc.ShaderType = 2 = SHADER_TYPE_PIXEL
```

mantendo a mesma infraestrutura de macros e source factory.

Fluxo confirmado:

```text
resources.json
   -> vertex / fragment paths
   -> macros JSON + macros internas
   -> compatibilidade de backend
   -> Diligent ShaderCreateInfo (HLSL)
   -> CreateShader(VS)
   -> CreateShader(PS)
   -> backend D3D / Vulkan / OpenGL compila a representação adequada
```

No Vulkan, a Diligent v2.5.4 usa sua infraestrutura glslang/SPIR-V; no OpenGL, a mesma source HLSL pode ser convertida/compilada pela infraestrutura Diligent.

## `textures`

Parser: `0x140087180`.

Campos confirmados:

```json
{
  "id": "...",
  "path": "...",
  "filter": "...",
  "wrap": "..."
}
```

Defaults definidos pelo parser antes dos overrides:

```text
filter = "linear"
wrap   = "repeat"
```

Strings relevantes:

```text
filter  0x140481D64
nearest 0x140481E00
repeat  0x140481E90
linear  0x140639D40
wrap    0x140639D48
```

`nearest` também pertence ao mesmo sistema de texturas e é referenciado por caminhos de carregamento. Ainda não declaramos uma enumeração completa de `wrap` enquanto todos os valores aceitos não forem provados.

Falha de carregamento: `failed to load texture ({})`.

## `models`

A coleção `models` é despachada para `0x140083A10`. O loader delega ao sistema de modelos e pode emitir `failed to load model ({})`. A descrição fina dos modelos pertence ao `model.json`, já documentado separadamente.

## Arquitetura útil para nosso MU

```text
resources.json
      |
      +-- attachments / IDs
      +-- shader programs + macros
      +-- textures + sampler policy
      +-- models
      v
resource registries
      v
render frontend / PSO / SRB
      v
OpenGL 4.6+ | Vulkan | D3D
```

Para nosso projeto, vale reproduzir essa separação: arquivos declarativos descrevem recursos; o backend apenas materializa buffers, texturas, shaders e pipelines.
