# 36 — Skeleton Texture / GPU skinning do NextMU

Status: **SOURCE-CORRELATED** com `mu_skeletonmanager.*`, `mu_math.h`, `mu_skeletoninstance.cpp` e Model bindings históricos; consistente com `g_SkeletonTexture` e `BoneOffset` recuperados do binário.

## Texture global de bones

```text
Width  = 2048
Height = 512
Format = RGBA32_FLOAT
Usage  = DEFAULT
Bind   = SHADER_RESOURCE
```

Memória:

```text
2048 * 512 * 16 bytes = 16 MiB
```

## Packing de cada bone

O NextMU deliberadamente não usa uma `mat4` por bone.

```cpp
struct NCompressedMatrix
{
    glm::quat Rotation; // 16 B
    glm::vec3 Position; // 12 B
    float Scale;        //  4 B
};                      // 32 B
```

Cada bone = **32 bytes = 2 texels RGBA32F**.

Não é dual quaternion; a própria source deixa isso explícito.

## Capacidade

```text
MaxBonesCount = (2048 * 512) / 2
              = 524,288 bones
```

O comentário original da arquitetura estima aproximadamente 2.600 personagens com 200 bones cada.

## Staging CPU

Existe um array global CPU:

```text
BonesBuffer[MaxBonesCount]
atomic BonesCount
```

A cada frame:

```text
Reset() -> BonesCount = 0
```

Cada `NSkeletonInstance::Upload()` reserva atomicamente uma faixa:

```text
index = BonesCount.fetch_add(bonesCount)
copy bones -> BonesBuffer[index]
return index
```

Esse `index` é o offset de bones usado pelo Model (`BoneOffset`).

## Upload GPU

No `Update()`:

```text
pixelsCount = bonesCount * 2
width  = min(pixelsCount, 2048)
height = ceil(pixelsCount / 2048)
```

O cliente faz um único `UpdateTexture` da região utilizada da skeleton texture e depois transiciona a texture para:

```text
RESOURCE_STATE_SHADER_RESOURCE
```

## Model binding

`g_SkeletonTexture` é **STATIC** no PSO do Model.

Por draw, `ModelSettings.BoneOffset` informa de qual posição da texture aquele personagem/modelo deve começar a ler.

Assim:

```text
Skeleton global texture = binding estável
BoneOffset              = dado pequeno por draw
```

## Comparação com nosso OpenGL 4.6 atual

Nosso caminho de referência do Main GL4.6 usa `u_Bones[200]` por upload de uniform. O NextMU mostra uma evolução interessante:

```text
Main GL4.6:
per-draw uniform mat4 array

NextMU:
global texture de compressed transforms
+ offset por instância
```

## Recomendação para nosso GL4.6+/Vulkan

Vale testar uma fase futura `SkeletonTexture`/`BoneBuffer` depois de estabilizar o ModernBMD atual.

Opções:

```text
OpenGL 4.6: texture buffer / 2D RGBA32F / SSBO
Vulkan: storage buffer ou sampled/storage texture
```

Para máxima paridade cross-backend, começar com o mesmo contrato lógico do NextMU:

```text
quat + position + scale
```

e reconstruir transform no vertex shader.

Benefícios esperados:

- elimina `u_Bones[200]` por draw;
- reduz chamadas de uniform;
- aumenta número de characters suportados;
- facilita batching de characters;
- deixa o binding do skeleton estável durante muitos draws;
- prepara caminho para instancing/indirect no futuro.
