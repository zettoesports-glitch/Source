# 31 — ModelSettings e Model uniforms exatos

Status: **CONFIRMADO — source histórica + layout observado no EXE**.

A source histórica da build de 25/01/2024 define `NModelSettings` com layout de 96 bytes, compatível com as escritas observadas no assembly.

```cpp
struct NModelSettings
{
    glm::vec4 LightPosition;   // 0x00, 16 B
    glm::vec4 BodyLight;       // 0x10, 16 B
    glm::vec4 BodyOrigin;      // 0x20, 16 B

    float BoneOffset;          // 0x30
    float NormalScale;         // 0x34
    float EnableLight;         // 0x38
    float AlphaTest;           // 0x3C
    float PremultiplyAlpha;    // 0x40
    float WorldTime;           // 0x44
    float ZTestRef;            // 0x48
    float Dummy1;              // 0x4C

    glm::vec2 BlendTexCoord;   // 0x50, 8 B
    float Dummy2;              // 0x58
    float Dummy3;              // 0x5C
}; // 0x60 = 96 B
```

## Semântica

- `LightPosition`: direção/posição de luz usada pelo material/model shader.
- `BodyLight`: luz/tint do objeto; pode ser premultiplicada conforme material/config.
- `BodyOrigin`: origem usada por efeitos/material/model space.
- `BoneOffset`: offset da pose/skeleton no recurso de bones.
- `NormalScale`: ajuste de normal usado por variantes de render/material.
- `EnableLight`: habilita iluminação do mesh.
- `AlphaTest`: controle de alpha-test/cutout.
- `PremultiplyAlpha`: seleciona tratamento premultiplied-alpha.
- `WorldTime`: tempo global para animações/efeitos dependentes de tempo.
- `ZTestRef`: referência usada por variantes de teste/depth/material.
- `BlendTexCoord`: deslocamento de UV para blend/chrome/efeitos.

## Upload

O Model usa uma arena/pool de uniforms em páginas, não um buffer novo por draw:

```text
ModelViewProj : 128 B por slot × 1024 = 128 KB/página
ModelSettings :  96 B por slot × 1024 =  96 KB/página
```

A atualização de `ModelSettings` usa:

```text
MAP_WRITE
MAP_FLAG_DISCARD
```

O draw referencia a faixa/slot correspondente do uniform.

## Bindings do Model

Estáticos no PSO:

```text
cbCameraAttribs
cbLightAttribs
ModelViewProj
g_SkeletonTexture
ModelSettings
```

Por SRB/material:

```text
g_VertexTexture
g_Texture
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

## Aplicação ao nosso renderer

Criar `ModelFrameConstants`, `ModelDrawConstants` e uma arena circular/paginada. OpenGL 4.6 pode implementar isso com UBO ranges; Vulkan, com dynamic uniform offsets ou ring buffers. O frontend deve produzir o mesmo `ModelSettings` para ambos.
