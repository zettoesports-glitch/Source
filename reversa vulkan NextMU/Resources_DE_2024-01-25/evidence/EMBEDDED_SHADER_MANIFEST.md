# Manifesto de shaders/fontes identificados no NextMU.exe

Foram identificados 52 nomes de arquivos de shader/include embutidos no executável x64. A maior parte pertence ao DiligentFX.

```text
FullScreenTriangleVS.fx
FullScreenTriangleVSOutput.fxh
ShadowConversions.fx
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
CoarseInsctr.fx
ComputeMinMaxShadowMapLevel.fx
Extinction.fxh
InitializeMinMaxShadowMap.fx
InterpolateIrradiance.fx
LookUpTables.fxh
MarkRayMarchingSamples.fx
RayMarch.fx
ReconstructCameraSpaceZ.fx
RefineSampleLocations.fx
RenderCoordinateTexture.fx
RenderSampling.fx
RenderSliceEndPoints.fx
ScatteringIntegrals.fxh
SliceUVDirection.fx
Sun.fx
UnshadowedScattering.fxh
UnwarpEpipolarScattering.fx
UpdateAverageLuminance.fx
CombineScatteringOrders.fx
ComputeScatteringOrder.fx
ComputeSctrRadiance.fx
InitHighOrderScattering.fx
PrecomputeAmbientSkyLight.fx
PrecomputeCommon.fxh
PrecomputeNetDensityToAtmTop.fx
PrecomputeSingleScattering.fx
UpdateHighOrderScattering.fx
EpipolarLightScatteringFunctions.fxh
EpipolarLightScatteringStructures.fxh
ToneMapping.fxh
ToneMappingStructures.fxh
```

## Interfaces observadas

Os sources usam sintaxe HLSL-like e incluem exemplos de:

```text
cbuffer
Texture2D
Texture2DArray
TextureCube
SamplerState
RWTexture2D
SV_Position
SV_Target
```

Também há caminhos para PBR, environment map convolution/prefilter, BRDF LUT, shadows e atmosfera/epipolar scattering.

## Observação

A presença desses sources no EXE confirma que a build inclui componentes DiligentFX. Não significa necessariamente que todo efeito esteja ativo no gameplay do NextMU ao mesmo tempo.
