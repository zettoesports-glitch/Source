# AUDITORIA COMPLETA FASE 7.5 — ANIMAÇÃO, SKINNING, OPENGL E VULKAN

**Branch auditada:** modernization  
**Data:** 2026-09-07  
**Status:** AUDITORIA COMPLETA DO CAMINHO DE ANIMAÇÃO/SKINNING

---

# RESUMO EXECUTIVO

A arquitetura de referência dos shaders está consistente entre OpenGL e Vulkan:

```
uvec2 Bones
uint BoneIndex
BonesTexture
RequestBone(BoneIndex, BoneId)
```

O BMD original também fornece naturalmente dois IDs:

```
Vertex_t::Node -> posição
Normal_t::Node -> normal
```

Porém a auditoria encontrou um bloqueador crítico na implementação já iniciada:

> **O SkeletonBuffer Matrix4x4 criado anteriormente não reproduz corretamente o layout CPU 3x4 do BMD quando simplesmente acrescenta uma quarta linha (0,0,0,1).**

O motivo é convenção de matriz/layout. O CPU usa matrizes 3x4 aplicadas por `VectorTransform`; o shader de referência reconstrói uma `mat4` específica e a usa com multiplicação vetorial própria. A transposição/layout precisa ser validada explicitamente.

Além disso, o shader OpenGL ativo de referência usa 2 texels/bone (Quaternion + PositionScale), enquanto o runtime BMD calcula primeiro transformações hierárquicas globais. Portanto não é seguro alimentar diretamente o shader com os quaternions locais de `BoneQuaternion[]`.

**Conclusão:** congelar a integração runtime e corrigir o encoder antes de compilar.

---

# 1. PIPELINE REAL DE ANIMAÇÃO BMD

## 1.1 Interpolação

Para cada bone:

```
q1/q2 dos keyframes
        ↓
QuaternionSlerp
        ↓
BoneQuaternion[i]

Position1/Position2
        ↓
interpolação linear
        ↓
Matrix local
```

Há tratamento especial para:

- BoneHead;
- LockPositions;
- BodyHeight.

## 1.2 Hierarquia

A transformação local não é enviada diretamente ao resultado final.

Root:

```
R_ConcatTransforms(ParentMatrix, LocalMatrix, BoneMatrix[i])
```

Child:

```
R_ConcatTransforms(BoneMatrix[parent], LocalMatrix, BoneMatrix[i])
```

### Veredito H1

`BoneMatrix[i]` final é uma transformação hierárquica/global acumulada.

**Consequência:** `BoneQuaternion[i]` isolado NÃO é suficiente para alimentar o shader de referência quando o bone possui parent transformado.

---

# 2. POSIÇÃO E ESCALA

## 2.1 ParentMatrix

Para root bones, `ParentMatrix` incorpora:

- BodyAngle;
- opcionalmente BodyScale;
- opcionalmente BodyOrigin.

Isso ocorre quando `Translate` está ativo.

## 2.2 Transform CPU

O caminho CPU depois aplica novamente lógica de:

- `_Scale`;
- `BoneScale`;
- `BodyScale`;
- `BodyOrigin`.

Portanto escala/origem não são exclusivamente dados do skeleton.

### Veredito S1

A arquitetura GPU deve manter separados:

```
Skeleton transform
Object BodyScale
Object BodyOrigin
Request/BoneScale quando semanticamente necessário
```

Não colocar BodyScale/BodyOrigin automaticamente dentro do BonesTexture.

---

# 3. VERTEX CONTRACT

Confirmado:

```
Vertex_t::Node  -> bone da posição
Normal_t::Node  -> bone da normal
```

Os shaders OpenGL e shadow usam:

```
layout(location = 3) in uvec2 input_Bones;
```

### Veredito V1

A alteração para:

```
GLuint m_iBones[2];
```

está conceitualmente correta.

### Risco V2

O runtime atual ainda precisa verificar todos os consumers de `VertexBMD` e os layouts/stride do projeto Visual Studio antes de marcar como integrado.

---

# 4. BONECONTAINER

O `ExtendVertex()` cria uma coleção de bones usados pela mesh.

A auditoria mostra que o caminho legado de palette ainda trabalha com:

```
BoneContainer
BuildModelBonePalette
m_BonePalette
SendBonePalette
u_Bones
```

### Veredito B1

Existem atualmente dois contratos em transição:

1. contrato legado baseado em palette/uniform;
2. contrato novo baseado em BoneId + BoneIndex + BonesTexture.

Eles não devem coexistir misturando índices.

### Decisão

O novo caminho deve usar Bone IDs reais e `BoneIndex` como base da allocation.

O caminho legado deve continuar isolado até a troca completa.

---

# 5. SEND BONE PALETTE

O caminho atual usa `shared_ptr<vector<float>>` e cache por identidade.

### Problema P1

Identidade do objeto não equivale à versão do conteúdo.

### Problema P2

A palette possui 12 floats por bone (3x4), enquanto o shader de referência possui layouts de:

- 8 floats/bone no modo QuaternionPositionScale;
- 16 floats/bone no modo Matrix4x4.

### Veredito P3

`SendBonePalette()` não pode ser reutilizado diretamente para BonesTexture.

Ele deve ser tratado como backend legado temporário.

---

# 6. SHADER OPENGL — MODELOS

Todos os principais shaders auditados seguem o mesmo contrato de vertex:

```
location 0 Position
location 1 UV
location 2 Normal
location 3 uvec2 Bones
location 6 BodyOrigin
location 7 BodyScale
...
location 12 uint BoneIndex
```

Fluxo:

```
RequestBone(BoneIndex, Bones.x)
    ↓
Position transform

RequestBone(BoneIndex, Bones.y) se diferente
    ↓
Normal transform
```

Depois:

```
Position += Normal * BodyScale.y
Position *= BodyScale.x
Position += BodyOrigin
```

### Veredito O1

`BodyScale` possui duas semânticas no shader de referência:

- x = escala do corpo;
- y = offset/influência ao longo da normal.

Isso precisa ser respeitado pelo instance data, não pelo SkeletonBuffer.

---

# 7. SHADER OPENGL — SOMBRAS

O shader de shadow usa o MESMO contrato:

```
uvec2 Bones
BoneIndex
BonesTexture
RequestBone()
```

A diferença principal é a matriz final:

```
Model -> Game3D
Shadow -> Shadow matrix
```

### Veredito O2

Models e Shadows devem compartilhar:

- SkeletonBuffer;
- BonesTexture;
- BoneIndex;
- IDs de bones.

Não criar skeleton específico para shadow.

---

# 8. VULKAN

O helper Vulkan confirma:

```
Texture2D<float4> / Buffer<float4>
RequestBone()
```

e suporta:

## Matrix mode

```
boneIndex *= 4
Load 4 float4
```

## QuaternionPositionScale mode

```
boneIndex *= 2
Load quaternion
Load position + scale
```

### Veredito VK1

O contrato lógico é compatível com OpenGL.

A diferença é apenas o recurso físico:

```
OpenGL -> sampler2D RGBA32F
Vulkan -> Texture2D ou Buffer<float4>
```

### Decisão VK2

O frontend `SkeletonBuffer` é correto conceitualmente, mas seu encoder/layout precisa ser corrigido antes da integração dos dois backends.

---

# 9. DESCOBERTA CRÍTICA — QUATERNION LOCAL VS TRANSFORM GLOBAL

O shader QuaternionPositionScale precisa representar a transformação que realmente deforma o vertex.

Mas o BMD calcula:

```
Local Quaternion + Local Position
        ↓
hierarquia
        ↓
Global BoneMatrix
```

Portanto:

```
BoneQuaternion[i]
```

não pode ser enviado sozinho para o GPU quando o bone possui parent.

### Solução correta

Encoder recebe o `BoneMatrix[i]` GLOBAL FINAL e produz:

```
Global affine transform
        ↓
decompose:
    global rotation
    global translation
    uniform scale
        ↓
QuaternionPositionScale
```

ou usar Matrix4x4 sem decomposição.

---

# 10. DESCOBERTA CRÍTICA — MATRIX LAYOUT

O `SkeletonBuffer::UploadAffine3x4()` criado anteriormente copia:

```
12 floats
+ (0,0,0,1)
```

Isso ainda não prova compatibilidade.

A ordem correta depende de:

- row-major CPU;
- GLSL mat4 column construction;
- transposição do shader gerado;
- convenção de `VectorTransform`.

### Veredito M1

O modo Matrix4x4 deve ser considerado **EXPERIMENTAL/BLOQUEADO** até existir teste matemático CPU vs shader.

Não integrar ao runtime.

---

# 11. TESTE DE VALIDAÇÃO OBRIGATÓRIO

Criar teste independente para cada bone:

## Position

```
CPU:
VectorTransform(vertex, BoneMatrix)

GPU-equivalent:
RequestBone(encodedBone) * vec4(vertex, 1)
```

## Normal

```
CPU:
VectorRotate(normal, BoneMatrix)

GPU-equivalent:
mat3(RequestBone(...)) * normal
```

Comparar:

```
abs(cpu - gpu) < epsilon
```

Casos obrigatórios:

1. root sem rotação;
2. root com BodyAngle;
3. child com parent rotacionado;
4. múltiplos níveis;
5. BodyScale;
6. BoneScale;
7. Translate false;
8. Translate true;
9. PositionBone != NormalBone;
10. LockPositions.

---

# 12. STATUS DOS ARQUIVOS CRIADOS

| Componente | Status |
|---|---|
| VertexBMD Bones[2] | CRIADO — auditoria pendente de compile |
| VAO uvec2 | CRIADO — auditoria pendente de compile |
| SkeletonBuffer frontend | CRIADO — layout Matrix bloqueado |
| OpenGLSkeletonTexture | CRIADO — backend físico não integrado |
| DualQuaternion StorageMode | CRIADO — encoder ausente |
| SendBonePalette legado | INTEGRADO runtime legado |
| OpenGL runtime novo | NÃO INTEGRADO |
| Vulkan runtime | NÃO INTEGRADO |

---

# 13. DECISÃO ARQUITETURAL FINAL

## Não fazer agora

- não integrar SkeletonBuffer ao renderer;
- não remover CPU fallback;
- não remover SendBonePalette;
- não alimentar BonesTexture com BoneQuaternion local;
- não assumir que 3x4 + linha extra é mat4 compatível;
- não marcar FASE 7 como concluída.

## Fazer agora

### Etapa A — Encoder validado

Criar:

```
SkeletonEncoder
```

com dois modos:

```
EncodeMatrix4x4(global BoneMatrix)
EncodeQuaternionPositionScale(global BoneMatrix)
```

### Etapa B — Math validation

Criar testes CPU que reproduzam exatamente as duas convenções.

### Etapa C — escolher formato inicial

Recomendação:

```
Matrix4x4 primeiro
```

porque evita decomposição e preserva exatamente a transformação global, MAS somente após validar layout.

### Etapa D

Depois do Matrix4x4 validado:

1. OpenGL BonesTexture;
2. BoneIndex instance attribute;
3. shader Matrix4x4 variant;
4. model;
5. shadow;
6. compilação;
7. teste visual.

### Etapa E

Depois:

```
QuaternionPositionScale
```

como otimização de memória/banda.

---

# VEREDITO FINAL

A auditoria confirma que a direção geral está correta, mas a integração ainda não deve começar.

O maior erro que evitamos foi enviar `BoneQuaternion[]` local diretamente ao shader: isso quebraria hierarquia.

O segundo maior risco é layout/transposição da matriz 3x4.

A arquitetura segura é:

```
BMD Animation
    ↓
Global BoneMatrix (já hierárquico)
    ↓
SkeletonEncoder VALIDADO
    ↓
SkeletonBuffer
    ↓
OpenGL / Vulkan backend
    ↓
RequestBone()
```

**FASE 7 permanece em desenvolvimento. Nenhum componente experimental deve ser declarado integrado até passar pelo teste matemático e compilação.**
