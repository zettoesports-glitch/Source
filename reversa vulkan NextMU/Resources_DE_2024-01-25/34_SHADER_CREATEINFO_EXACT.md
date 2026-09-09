# 34 — `ShaderCreateInfo` exato usado pelo NextMU

Status: **CONFIRMADO** por assembly de `0x140084530` cruzado com `ShaderCreateInfo` da Diligent Engine v2.5.4.

## Estrutura preenchida

Base local observada em torno de `rbp+0x90`.

O NextMU inicializa o `ShaderCreateInfo` aproximadamente assim:

```cpp
ShaderCreateInfo CI{};
CI.FilePath                     = nullptr;
CI.pShaderSourceStreamFactory   = nullptr;
CI.Source                       = source_in_memory;
CI.ByteCode                     = nullptr;
CI.SourceLength                 = 0;
CI.EntryPoint                   = "main";
CI.Macros.Elements              = macros.data();
CI.Macros.Count                 = macros.size();
CI.Desc.Name                    = nullptr;
CI.Desc.UseCombinedTextureSamplers = true;
CI.Desc.CombinedSamplerSuffix   = "_sampler";
CI.SourceLanguage               = SHADER_SOURCE_LANGUAGE_HLSL;
CI.ShaderCompiler               = SHADER_COMPILER_DEFAULT;
CI.HLSLVersion                  = {};
CI.GLSLVersion                  = {};
CI.GLESSLVersion                = {};
CI.MSLVersion                   = {};
CI.CompileFlags                 = SHADER_COMPILE_FLAG_NONE;
CI.LoadConstantBufferReflection = false;
```

### Vertex stage

Antes do primeiro `CreateShader`:

```text
CI.Desc.ShaderType = 1
```

Na Diligent v2.5.4:

```text
1 = SHADER_TYPE_VERTEX
```

Call:

```text
0x140085BBA -> IRenderDevice::CreateShader
```

### Fragment/Pixel stage

Antes da segunda chamada:

```text
CI.Desc.ShaderType = 2
```

Na Diligent v2.5.4:

```text
2 = SHADER_TYPE_PIXEL
```

Call:

```text
0x140085DCE -> IRenderDevice::CreateShader
```

## Strings fixas

```text
0x1405F3564 "main"
0x1405F3558 "_sampler"
```

## Source é carregado para memória

O `FilePath` do `ShaderCreateInfo` permanece nulo neste estágio.
O source processado é colocado diretamente em:

```text
ShaderCreateInfo::Source
```

Portanto o fluxo é:

```text
resource/attachment
     ↓
carregar texto
     ↓
normalizar / aplicar macros
     ↓
ShaderCreateInfo.Source
     ↓
CreateShader
```

Isso também significa que o Diligent não precisa conhecer o caminho físico original do shader no momento de compilação.

## Linguagem fonte — HLSL

O assembly grava:

```text
SourceLanguage = 1
```

Na Diligent v2.5.4:

```text
0 DEFAULT
1 HLSL
2 GLSL
3 GLSL_VERBATIM
...
```

Logo os shaders próprios desta pipeline são fornecidos ao Diligent como **HLSL**.

Essa decisão é importante para o backend comum:

```text
HLSL source
   ├─ D3D11/D3D12 -> HLSL compiler
   ├─ Vulkan      -> HLSL -> SPIR-V
   └─ OpenGL      -> HLSL -> GLSL -> driver
```

## Combined samplers

O cliente ativa:

```cpp
UseCombinedTextureSamplers = true;
CombinedSamplerSuffix = "_sampler";
```

Portanto uma texture `g_Texture` pode ser associada ao sampler correspondente usando a convenção:

```text
g_Texture_sampler
```

Isso simplifica a mesma source entre APIs que têm modelos diferentes de texture/sampler.

## Normalização específica para OpenGL/GLES

Helper:

```text
0x140082C70
```

A função consulta o `RENDER_DEVICE_TYPE`. Para os valores:

```text
3 = RENDER_DEVICE_TYPE_GL
4 = RENDER_DEVICE_TYPE_GLES
```

ela passa o source por substituições de tipos:

```text
half4 -> float4
half3 -> float3
half2 -> float2
half  -> float
```

Strings diretamente presentes:

```text
float4 / half4
float3 / half3
float2 / half2
float  / half
```

Isso é uma camada de compatibilidade para o caminho HLSL -> GLSL.

## Macros

O vetor de `ShaderMacro` usa elementos de **16 bytes** em x64:

```cpp
struct ShaderMacro {
    const char* Name;
    const char* Definition;
};
```

Além das macros por programa vindas do JSON, o cliente adiciona as macros globais já documentadas em `33_RESOURCES_JSON_AND_SHADER_SCHEMA.md`.

## O que isso nos dá para o nosso MU

Uma arquitetura de shader compartilhada pode ser reproduzida com alta fidelidade:

```text
HLSL canonical source
        ↓
macro variant builder
        ↓
backend normalization
        ↓
GL4.6+ / Vulkan compiler path
```

Não precisamos manter um shader GLSL e outro shader Vulkan separados para cada efeito. O frontend pode manter um contrato único e gerar o target de cada backend.