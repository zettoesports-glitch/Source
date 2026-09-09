# 11 — Shader library e pipeline cross-API

Status: **CONFIRMADO por strings/source paths do NextMU e GraphicsEngineVk.**

## Compiladores presentes no backend Vulkan

A DLL Vulkan contém módulos:

```text
GLSLangUtils.cpp
DXCompiler.cpp
HLSLUtils.hpp
SPIRVShaderResources.cpp
ShaderToolsCommon.cpp
GLSLUtils.cpp
HLSL2GLSLConverterImpl.cpp
```

Capacidades comprovadas:

```text
HLSL -> SPIR-V
GLSL -> SPIR-V
SPIR-V reflection/resource parsing
HLSL -> GLSL
```

Consequência: o jogo pode manter uma source de shader comum e deixar o backend gerar o formato esperado por Vulkan/OpenGL/D3D.

## DiligentFX embutido no NextMU

Arquivos identificados:

```text
FullScreenTriangleVSOutput.fxh
AtlasSampling.fxh
BasicStructures.fxh
PBR_Common.fxh
ShaderUtilities.fxh
Shadows.fxh
ComputeIrradianceMap.psh
CubemapFace.vsh
GLTF_PBR_PrecomputeCommon.fxh
PrecomputeGLTF_BRDF.psh
PrefilterEnvMap.psh
RenderGLTF_PBR.psh
RenderGLTF_PBR.vsh
GLTF_PBR_Shading.fxh
GLTF_PBR_Structures.fxh
GLTF_PBR_VertexProcessing.fxh
AtmosphereShadersCommon.fxh
LookUpTables.fxh
ScatteringIntegrals.fxh
Extinction.fxh
UnshadowedScattering.fxh
ToneMapping.fxh
PrecomputeCommon.fxh
EpipolarLightScatteringFunctions.fxh
EpipolarLightScatteringStructures.fxh
ToneMappingStructures.fxh
```

## O que isso oferece para nosso MU

```text
MU shader source
    ├── Vulkan -> SPIR-V
    ├── OpenGL -> GLSL
    └── futuro D3D -> HLSL/DXIL/DXBC
```

Isso reduz duplicação entre nosso OpenGL 4.6+ e Vulkan.

Features visuais para estudo:

- PBR/BRDF;
- prefiltered environment map;
- irradiance map;
- shadow helpers;
- fullscreen passes;
- atmosphere/scattering;
- tone mapping.

Primeiro preservar o visual MU clássico; depois adicionar modos opcionais.

## Recomendação para nosso shader system

```text
/common
    math
    lighting
    fog
    material
    skinning
    shadow
/model
/terrain
/effects
/ui
/post
```

Uma lista comum de resources por programa, com glue específico apenas no backend.

## Pendência de fidelidade

Os arquivos DiligentFX ainda serão comparados com a tag pública v2.5.4 para detectar customizações. A presença no binário não prova que cada shader esteja ativo em todas as cenas.
