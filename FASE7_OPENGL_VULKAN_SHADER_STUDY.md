# FASE 7 — Estudo comparativo completo dos shaders OpenGL e Vulkan

## Escopo estudado

### Fonte Vulkan/HLSL
`vulkan-main/vulkan-main/Shaders`

Arquivos-base analisados:
- `common/VertexDefinitions.inc`
- `common/GlobalConstantBuffers.inc`
- `common/SkeletonHelpers.inc`
- `models/texture.vs`
- família `models/`
- família `shadows/`

### Backend OpenGL/GLSL
`opengl-main/opengl-main/OpenGL`

Arquivo comparado diretamente:
- `models/texture.vs`

O levantamento anterior confirma 176 vertex shaders e 64 pixel shaders no OpenGL, contra 45 vertex shaders, 17 pixel shaders e 8 includes na árvore Vulkan/HLSL.

---

# 1. Conclusão principal

Os dois backends seguem o MESMO contrato lógico de renderização.

O HLSL da árvore Vulkan é a fonte legível de referência. O GLSL OpenGL é gerado/cross-compilado dessa lógica.

Não devemos manter duas implementações independentes de skinning.

```
Shared Engine Contract
        |
        +-------------------+
        |                   |
     OpenGL               Vulkan
        |                   |
      GLSL            HLSL -> SPIR-V
```

---

# 2. Vertex contract real

```
location 0  float3 Position
location 1  float2 UV
location 2  float3 Normal
location 3  uint2  Bones
location 6  float3 BodyOrigin
location 7  float2 BodyScale
location 8  float2 Data
location 9  float4 Data2
location 10 float4 BodyLight
location 11 float4 ShadowLight
location 12 uint   BoneIndex
```

A semântica de `Bones` é explicitamente:

```
Bones.x = bone usado para posição
Bones.y = bone usado para normal
```

`BoneIndex` é por instância e representa a base da palette/skeleton daquela instância.

## Decisão

A FASE 7 não deve mudar o BMD para quatro influências genéricas como contrato principal.

O formato compatível com os shaders existentes é:

```
uvec2 Bones + uint BoneIndex por instância
```

---

# 3. Skeleton transport

O caminho de referência não usa array UBO fixo de 200 bones.

Usa `BonesTexture`.

Vulkan/HLSL:

```
Buffer<float4> BonesTexture
ou
Texture2D<float4> BonesTexture
```

OpenGL:

```
sampler2D BonesTexture
texelFetch(...)
```

O endereço físico é calculado com `SKELETON_TEXTURE_WIDTH`.

Cada bone usa:

- 4 texels no modo matriz;
- 2 texels no modo dual quaternion.

```
CPU Skeleton
    |
    v
Shared SkeletonBuffer
    |
    +--------------------+
    |                    |
 OpenGL Texture      Vulkan Buffer/Image
    |                    |
 sampler2D        Buffer/Texture descriptor
    |                    |
    +-------- BonesTexture --------+
                 |
                 v
            RequestBone()
```

---

# 4. Skinning real do modelo

O shader `models/texture.vs` executa:

1. `RequestBone(BoneIndex, Bones.x)`
2. transforma a posição;
3. se `Bones.x != Bones.y`, busca outro bone para normal;
4. transforma normal;
5. aplica BoneScale;
6. aplica BodyScale;
7. soma BodyOrigin;
8. aplica `Game3D`.

Isso também explica por que sombras usam o mesmo contrato de skeleton.

---

# 5. Shadows

Os shaders de `shadows/` reutilizam:

- `VertexDefinitions.inc`
- `SkeletonHelpers.inc`
- `GlobalConstantBuffers.inc`

Portanto, qualquer implementação de GPU skinning precisa alimentar modelo e shadow pass com o mesmo `BonesTexture`.

Não basta corrigir apenas `models/`.

---

# 6. Global constants

O layout compartilhado contém:

- ShadowColor
- ShadowPosition
- Shadow
- Game3D
- Game2D
- BillboardQ
- LightPosition
- WorldTime
- ChromeWave
- Random
- Wave
- WaterMove
- WindScale
- WindSpeed
- WaterIndex

Esse layout já é o contrato comum OpenGL/Vulkan.

---

# 7. Correção de arquitetura da FASE 7

Infraestrutura experimental criada anteriormente:

```
BoneUBO[200]
BoneIndices[4]
BoneWeights[4]
```

não deve ser promovida como contrato final sem adaptação.

O contrato real esperado pelos shaders do repositório é:

```
Vertex BMD:
  Position
  UV
  Normal
  Bones[2]

Instance:
  BoneIndex

Skeleton:
  BonesTexture
```

## Caminho correto

1. preservar CPU fallback;
2. mapear Node/Bone original do BMD para `Bones.x` e `Bones.y`;
3. criar `SkeletonBuffer` compartilhado;
4. backend OpenGL: texture/texelFetch;
5. backend Vulkan: buffer ou image descriptor;
6. alimentar `BoneIndex` por instância;
7. integrar modelo e shadows;
8. somente então habilitar GPU skinning runtime.

---

# 8. Estado atual

| Item | Status |
|---|---|
| Estudo Vulkan/HLSL | CONCLUÍDO |
| Estudo OpenGL/GLSL | CONCLUÍDO |
| Contrato compartilhado identificado | CONCLUÍDO |
| BonesTexture identificado | CONCLUÍDO |
| Modelo + Shadows mapeados | CONCLUÍDO |
| Arquitetura final da FASE 7 definida | CONCLUÍDA |
| SkeletonBuffer C++ | PRÓXIMO |
| Backend OpenGL | PRÓXIMO |
| Backend Vulkan | PRÓXIMO |
| Runtime integration | PENDENTE |
| Build | AINDA NÃO |

## Decisão final

A implementação continuará seguindo o contrato existente dos shaders OpenGL/Vulkan, e não uma nova convenção incompatível.
