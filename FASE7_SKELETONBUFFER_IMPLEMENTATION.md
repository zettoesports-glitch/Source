# FASE 7 — SkeletonBuffer Frontend

## Estado

**CRIADO — ainda não integrado ao runtime**

Este componente substitui a ideia de um BoneUBO fixo por um frontend backend-neutral compatível com o contrato real dos shaders:

```
RequestBone(BoneIndex, BoneId)
```

## Contrato

Cada allocation recebe:

- `BaseBone`: enviado por instância como `BoneIndex`;
- `BoneCount`;
- `Generation`: impede reutilização de allocation de frame antigo.

O mesh contém Bone IDs locais:

```
Bones.x = position bone
Bones.y = normal bone
```

No shader:

```
absoluteBone = BoneIndex + BoneId
``

## Layout

Modo matrix atual:

```
Bone 0:
  texel 0 = matrix row 0
  texel 1 = matrix row 1
  texel 2 = matrix row 2
  texel 3 = affine homogeneous row
```

Total: 4 float4 texels por bone, compatível com `SkeletonHelpers.inc`.

## Próximos passos

1. adicionar os arquivos ao projeto Visual Studio;
2. conectar a fonte real de BoneMatrix;
3. criar backend OpenGL para upload de `GetTexels()`;
4. passar `Allocation.BaseBone` como `BoneIndex`;
5. integrar model + shadow;
6. validar visualmente;
7. criar backend Vulkan equivalente.
