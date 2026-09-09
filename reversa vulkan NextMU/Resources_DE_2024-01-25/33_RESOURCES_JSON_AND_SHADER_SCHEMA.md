# 33 — `resources.json` e schema de shaders

Status: **CONFIRMADO** por strings + xrefs + assembly do `NextMU.exe` x64.

## Loader principal

Função principal:

```text
0x140083050
```

O cliente abre:

```text
data/resources.json
```

Strings:

```text
0x140484908  data/
0x140484910  resources.json
0x140484920  resources.json missing ({})
0x140484940  resources malformed ({})
```

## Seções do arquivo

O JSON possui seções independentes:

```text
attachments
shaders
textures
models
```

Dispatch observado:

```text
attachments -> chamada em torno de 0x14008CD30
shaders     -> loader 0x140086250
textures    -> loader 0x140087180
models      -> loader 0x140083A10
```

Isso confirma que o frontend mantém registries separados de resources.

---

# Shader definitions

Loader da seção `shaders`:

```text
0x140086250
```

Campos confirmados:

```text
vertex
fragment
resource_id
macros
```

VAs:

```text
0x140484A50 vertex
0x140484A58 resource_id
0x140484A64 macros
```

`fragment` é construído inline no assembly do parser em torno de `0x140086584`.

Depois de interpretar cada entrada, o loader chama a criação central:

```text
0x140084530
```

---

# Macros globais injetadas pelo cliente

Antes das macros particulares do shader, o helper de criação injeta:

```text
SKELETON_TEXTURE_WIDTH  = 2048
SKELETON_TEXTURE_HEIGHT = 512
USE_SHADOW              = configuração/runtime bool
SHADOW_MODE             = configuração/runtime integer
SHADOW_FILTER_SIZE      = configuração/runtime integer
FILTER_ACROSS_CASCADES  = configuração/runtime bool
BEST_CASCADE_SEARCH     = configuração/runtime bool
```

Strings e VAs:

```text
0x1404849B0 SKELETON_TEXTURE_WIDTH
0x1404849C8 SKELETON_TEXTURE_HEIGHT
0x1404849E0 USE_SHADOW
0x1404849F0 SHADOW_MODE
0x140484A00 SHADOW_FILTER_SIZE
0x140484A18 FILTER_ACROSS_CASCADES
0x140484A30 BEST_CASCADE_SEARCH
```

Helpers consultados para shadows:

```text
0x140052950 USE_SHADOW
0x1400529B0 SHADOW_MODE
0x1400529A0 SHADOW_FILTER_SIZE
0x140052990 FILTER_ACROSS_CASCADES
0x140052960 BEST_CASCADE_SEARCH
```

## Skeleton texture

Os valores `0x800` e `0x200` são gravados diretamente como macros. Portanto a configuração desta build usa:

```text
2048 x 512
```

para a skeleton texture.

---

# Schema de uma macro configurável

Cada entrada em `macros` contém:

```text
type
name
value
```

Tipos textuais confirmados:

```text
integer
float
boolean
string
```

Strings observadas:

```text
0x140484A48 integer
0x140639DC8 float
0x140481270 boolean
0x140639DD8 string
0x14048028C value
```

`type` e `name` também são montados inline no parser.

O cliente possui paths separados para converter/adicionar integer, float, boolean e string ao vetor de macros enviado ao compilador.

Helpers observados:

```text
0x14007FCA0  inclusão/conversão de macro inteira
0x14007FB00  inclusão/conversão de macro numérica/booleana
```

---

# Criação real dos shaders

A função `0x140084530` constrói estruturas Diligent para os dois stages e chama `IRenderDevice::CreateShader` duas vezes:

```text
~0x140085BBA
~0x140085DCE
```

Na ABI Diligent v2.5.4, `IRenderDevice +0x28` é `CreateShader`.

As duas chamadas correspondem aos shaders `vertex` e `fragment` da definição carregada.

O vetor de macros é passado ao `ShaderCreateInfo` e cada macro ocupa um par nome/definição de 16 bytes na ABI x64.

---

# Consequência arquitetural

O NextMU não precisa recompilar C++ para gerar todas as variantes de shader.

```text
resources.json
   ↓
vertex + fragment
   ↓
macros globais do renderer
   +
macros específicas da entrada
   ↓
Diligent CreateShader
   ↓
HLSL/GLSL -> backend target
   ↓
D3D11 / Vulkan / D3D12 / OpenGL
```

Isso é uma ótima referência para nosso GL4.6+/Vulkan: usar uma única definição de programa e compilar variantes controladas por configuração.

## Ainda em investigação

- layout completo de `ShaderCreateInfo` usado pela build;
- linguagem fonte exata e flags;
- convenção de caminho dos arquivos vertex/fragment;
- entry point;
- combined texture samplers;
- formato exato do objeto externo em `resources.json["shaders"]`.

Esses itens só serão promovidos para CONFIRMADO depois de cruzar `0x140084530` com `ShaderCreateInfo` da Diligent v2.5.4.