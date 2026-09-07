# ESTUDO PROFUNDO — Dual Quaternion no pipeline BMD

## Veredito

O projeto já contém a resposta principal: **não é necessário decompor a matriz final para descobrir a animação**.

A animação BMD já calcula, para cada osso:

1. quaternion interpolado em `BoneQuaternion[i]`;
2. matriz 3x4 equivalente em `BoneMatrix[i]`.

Trecho conceitual do fluxo atual:

```
QuaternionSlerp(q1, q2, s1, BoneQuaternion[i])
        ↓
QuaternionMatrix(BoneQuaternion[i], Matrix)
        ↓
posição interpolada
        ↓
hierarquia R_ConcatTransforms(...)
        ↓
BoneMatrix[i][3][4]
```

## Shader real

O shader OpenGL `models/texture.vs` usa:

```
uvec2 Bones
uint BoneIndex
sampler2D BonesTexture
USE_DUAL_QUATERNION
```

Para cada bone:

```
Texel 0 = quaternion (x,y,z,w)
Texel 1 = position (x,y,z) + uniform scale
```

O helper Vulkan confirma a mesma representação.

## Descoberta crítica: não é Dual Quaternion matemático clássico

O nome `USE_DUAL_QUATERNION` pode induzir ao erro.

O shader atual NÃO carrega:

```
real quaternion
dual quaternion
```

Ele carrega:

```
rotation quaternion
position.xyz + scale
```

Depois reconstrói uma matriz affine.

Portanto o formato do projeto é mais precisamente:

**Quaternion + Translation + Uniform Scale (QTScale)**

e não o dual quaternion clássico de 8 componentes.

## Consequência para o CPU

A implementação mais segura é converter a matriz affine 3x4 já calculada para:

```
rotation quaternion
translation
uniform scale
```

sem tocar no algoritmo de animação BMD.

### Extração

Para cada matriz M:

1. comprimento das três bases = escala;
2. remover escala da parte 3x3;
3. converter matriz de rotação normalizada para quaternion;
4. copiar translation M[0..2][3];
5. guardar a escala.

## Risco

O MU atual pode aplicar escala:

- no ParentMatrix;
- em BodyScale;
- via BoneScale;
- via m_fRequestScale.

Nem toda escala pertence ao skeleton.

**Não devemos converter para QTScale antes de separar escala de objeto da escala do osso.**

## Melhor arquitetura

### Skeleton animation

```
Quaternion + Translation
```

### Instance data

```
BodyScale
BodyOrigin
RequestScale
```

### Shader

```
RequestBone()
→ rotation quaternion
→ position
→ scale somente se existir escala real do bone
```

## Decisão

Não alterar ainda a animação.

Primeiro implementar um encoder isolado:

```
Affine3x4
   ↓
QuaternionPositionScale
   ↓
2 texels/bone
```

com testes CPU comparando:

```
VectorTransform(originalMatrix, vertex)
vs
Shader-equivalent reconstructedMatrix(vertex)
```

O erro deve ser próximo de zero para posição e normal antes da integração.
