# 3 — Pipeline de shaders: HLSL/GLSL -> SPIR-V

Esta é uma das partes mais valiosas do pacote para nosso projeto.

## Conclusão principal

O NextMU usa uma arquitetura em que o código de shader pode ser compartilhado entre múltiplos backends, enquanto cada backend compila/converte para sua representação nativa.

Na DLL Vulkan foram confirmados:

```text
Diligent::GLSLangUtils::HLSLtoSPIRV
Diligent::GLSLangUtils::GLSLtoSPIRV
Diligent::SPIRVShaderResources
Diligent::ShaderVkImpl::ShaderVkImpl
```

Source paths confirmados:

```text
DiligentCore/Graphics/ShaderTools/src/GLSLangUtils.cpp
DiligentCore/Graphics/ShaderTools/src/SPIRVShaderResources.cpp
DiligentCore/Graphics/ShaderTools/src/DXCompiler.cpp
DiligentCore/Graphics/ShaderTools/src/GLSLUtils.cpp
DiligentCore/Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp
DiligentCore/Graphics/GraphicsEngineVulkan/src/ShaderVkImpl.cpp
```

Também existem componentes glslang e SPIR-V Tools embutidos na build.

## Arquitetura recuperada

```text
Shader source comum
(HLSL ou GLSL)
        |
        +----------------------------+
        |                            |
        v                            v
D3D11/D3D12                     Vulkan
HLSL/compiler                   glslang
                                    |
                                    v
                                  SPIR-V
                                    |
                                    v
                         SPIRVShaderResources
                           reflection/layout
                                    |
                                    v
                            VkShaderModule
                                    |
                                    v
                         Graphics/Compute PSO
```

OpenGL também possui backend próprio e a build inclui conversor HLSL->GLSL.

## Shaders embutidos no NextMU.exe

O EXE contém uma grande biblioteca DiligentFX em fonte HLSL-like, incluindo:

```text
RenderGLTF_PBR.vsh
RenderGLTF_PBR.psh
ComputeIrradianceMap.psh
PrefilterEnvMap.psh
PrecomputeGLTF_BRDF.psh
CubemapFace.vsh
Shadows.fxh
ShadowConversions.fx
PBR_Common.fxh
AtlasSampling.fxh
ShaderUtilities.fxh
ToneMapping.fxh
AtmosphereShadersCommon.fxh
RayMarch.fx
Sun.fx
CoarseInsctr.fx
UnwarpEpipolarScattering.fx
PrecomputeSingleScattering.fx
```

O manifesto completo dos nomes identificados está em `evidence/EMBEDDED_SHADER_MANIFEST.md`.

## Evidência cross-API dentro dos shaders

Há condicionais como:

```text
#ifdef GLSL
#if (defined(GLSL) || defined(GL_ES)) && !defined(VULKAN)
```

E comentários distinguindo diferenças entre:

```text
Direct3D
Vulkan
OpenGL
```

Isso mostra que o mesmo shader source foi pensado para múltiplos backends.

## Por que isso é bom para nosso MU

Em vez de manter:

```text
Model_OpenGL.glsl
Model_Vulkan.spv/source separado
Model_D3D.hlsl
```

podemos caminhar para:

```text
Model.hlsl (fonte canônica)
    |
    +--> OpenGL: conversão/GLSL
    +--> Vulkan: SPIR-V
```

ou manter GLSL como fonte principal e produzir SPIR-V para Vulkan. O ponto importante é ter **um contrato único de uniforms/buffers/materials**.

## Recomendação prática

Para nosso projeto atual, eu não mudaria todos os shaders OpenGL já estabilizados de uma vez. Primeiro:

1. congelar layouts de `FrameData`, material, vertex e bones;
2. criar uma representação abstrata de shader/resource bindings;
3. portar um único pipeline BMD para Vulkan;
4. validar visualmente OpenGL x Vulkan;
5. só depois generalizar para terrain/2D/effects.

Assim aproveitamos a ideia boa do NextMU sem reescrever nosso renderer inteiro de uma vez.
