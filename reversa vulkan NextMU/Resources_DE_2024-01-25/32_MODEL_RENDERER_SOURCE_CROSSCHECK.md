# 32 — Model Renderer: cross-check source ↔ binário

Status: **CONFIRMADO POR SOURCE + BINÁRIO**.

## Uniforms exatos

```cpp
struct NModelViewSettings
{
    glm::mat4 Model;
    glm::mat4 ViewProj;
}; // 128 B

struct NModelSettings
{
    glm::vec4 LightPosition;   // 0x00
    glm::vec4 BodyLight;       // 0x10
    glm::vec4 BodyOrigin;      // 0x20
    float BoneOffset;          // 0x30
    float NormalScale;         // 0x34
    float EnableLight;         // 0x38
    float AlphaTest;           // 0x3C
    float PremultiplyAlpha;    // 0x40
    float WorldTime;           // 0x44
    float ZTestRef;            // 0x48
    float Dummy1;              // 0x4C
    glm::vec2 BlendTexCoord;   // 0x50
    float Dummy2;              // 0x58
    float Dummy3;              // 0x5C
}; // 96 B
```

## Uniform data por draw

`Model` recebe a model matrix; `ViewProj` usa a view-projection transposta do render state.

`NModelSettings` recebe terrain light position, body light/origin, bone offset, alpha test, premultiply alpha, world time, ZTestRef e blend texcoord.

`ZTestRef` observado na source:

```text
-3000.0
```

## Buffers

Os uniform buffers de 128 B e 96 B são `USAGE_DYNAMIC + CPU_ACCESS_WRITE` e atualizados com Map/Discard através do `NRenderManager`.

As filas CPU usam páginas/reuso em vez de alocar um objeto GPU por mesh.

## PSO e SRB

PSO estático:

```text
VS cbCameraAttribs
VS+PS cbLightAttribs
VS ModelViewProj
VS g_SkeletonTexture
VS+PS ModelSettings
```

SRB mutável:

```text
VS g_VertexTexture (opcional)
PS g_Texture
PS g_tex2DShadowMap ou g_tex2DFilterableShadowMap
```

O cache de binding considera o PipelineState e IDs dos recursos relevantes.

## Draw

O model path normal usa `Draw`, não `DrawIndexed`, porque a geometria é expandida para triangle list durante a conversão do modelo.

Sequência lógica:

```text
GetPipelineState
resolve/cria SRB
UpdateBufferWithMap(ModelViewProj)
UpdateBufferWithMap(ModelSettings)
SetPipelineState
SetVertexBuffer
CommitShaderResources
Draw
```

## Vertex layout

Formato não comprimido confirmado:

```text
Position  float3
Normal    float3
TexCoords float2
Bone      uint8[2]
Vertex    uint16
stride    36 B
```

A mesma estrutura é adequada para um frontend comum GL4.6+/Vulkan.
