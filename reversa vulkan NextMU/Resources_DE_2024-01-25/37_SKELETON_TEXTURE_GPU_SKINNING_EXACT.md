# 37 — Skeleton texture e GPU skinning exatos

Status: **CONFIRMADO — source histórica de 25/01/2024 + contratos observados no EXE**.

## Ideia central

O NextMU não envia uma matriz 4x4 por bone nem um array de bones por draw. Todos os skeletons visíveis do frame são compactados em uma única texture global e cada model draw recebe apenas um `BoneOffset`.

## Formato de bone

```cpp
struct NCompressedMatrix
{
    glm::quat Rotation; // 4 floats = 16 B
    glm::vec3 Position; // 3 floats = 12 B
    float Scale;        // 1 float  =  4 B
}; // 32 B
```

Não é dual-quaternion. É quaternion + position + uniform scale.

Transform CPU equivalente:

```text
position' = Rotation * (position * Scale) + Position
normal'   = Rotation * (normal   * Scale)
```

## Skeleton Texture

```text
Width  = 2048
Height = 512
Format = RGBA32_FLOAT
Usage  = DEFAULT
Bind   = SHADER_RESOURCE
VRAM   ≈ 16 MB
```

Cada bone contém 8 floats = 2 texels RGBA32F:

```text
texel 0 = Rotation.xyzw
texel 1 = Position.xyz + Scale
```

Capacidade:

```text
2048 × 512 = 1,048,576 texels
/ 2 texels por bone
= 524,288 bones
```

Comentário original da source estima cerca de **2600 personagens × 200 bones**.

## Allocator de bones por frame

CPU:

```text
std::vector<NCompressedMatrix> BonesBuffer
atomic uint32 BonesCount
```

No começo do frame:

```cpp
BonesCount = 0;
```

Cada skeleton chama:

```cpp
index = BonesCount.fetch_add(bonesCount);
memcpy(&BonesBuffer[index], bones, sizeof(NCompressedMatrix) * bonesCount);
return index;
```

Esse `index` vira `NRenderConfig::BoneOffset` e, depois, `ModelSettings.BoneOffset`.

O uso de `fetch_add` permite que o `PreRender` multithread de Characters/Objects faça reservas concorrentes sem allocator global com lock pesado.

## Upload GPU

Depois do PreRender, `MUSkeletonManager::Update()` calcula somente a região realmente ocupada:

```text
pixelsCount = bonesCount * 2
width       = min(pixelsCount, 2048)
height      = ceil(pixelsCount / 2048)
```

Então executa um único:

```text
UpdateTexture(BonesTexture, region occupied this frame)
```

com row stride:

```text
sizeof(vec4) * width
```

Depois restaura/atualiza o estado para `SHADER_RESOURCE`.

## Ordem no frame

```text
frame start
  ↓
MUSkeletonManager::Reset
  ↓
Characters::PreRender / Objects::PreRender
  ├─ animate skeleton
  ├─ Upload() → atomic BoneOffset
  └─ prepare model data
  ↓
MUSkeletonManager::Update
  └─ one batched texture upload
  ↓
Model draws
  └─ ModelSettings.BoneOffset indexes global skeleton texture
```

## Por que isso é muito importante para nosso MU

Substitui:

```text
muitos glUniformMatrix/glBufferSubData por mesh/player
```

por:

```text
1 pose allocator CPU
1 upload grande por frame
1 global bone resource
1 small BoneOffset por draw
```

### OpenGL 4.6+

Podemos implementar com:

- `GL_TEXTURE_2D` RGBA32F como baseline fiel;
- ou SSBO/TBO como evolução futura após validar compatibilidade/desempenho.

### Vulkan

- sampled RGBA32F texture igual ao NextMU;
- ou storage buffer como evolução posterior.

Recomendação: primeiro reproduzir o texture-based skeleton exatamente, medir, depois comparar SSBO.
