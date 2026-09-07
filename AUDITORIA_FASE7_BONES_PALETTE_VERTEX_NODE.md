# AUDITORIA PROFUNDA — SendBonePalette + Vertex_t::Node

**Fase:** Pós-auditoria FASE 7  
**Branch:** modernization  
**Data:** 2026-09-07

## 1. Objetivo

Auditar o caminho real atual:

```
Vertex_t::Node / Normal_t::Node
        ↓
VertexBMD::m_iBone
        ↓
VAO/VBO OpenGL
        ↓
CGMMeshShader::AddBoneTransform
        ↓
BuildModelBonePalette
        ↓
RenderMeshVAO::m_BonePalette
        ↓
SendBonePalette
        ↓
u_Bones uniform
```

e comparar com o contrato de referência OpenGL/Vulkan:

```
uvec2 Bones + uint BoneIndex + BonesTexture + RequestBone()
```

---

# 2. Vertex_t::Node — RESULTADO

## Código-fonte atual

```cpp
typedef struct
{
    short  Node;
    vec3_t Position;
} Vertex_t;

typedef struct
{
    short  Node;
    vec3_t Normal;
    short  BindVertex;
} Normal_t;
```

### Conclusão

O formato BMD atual contém DOIS índices semanticamente diferentes:

- `Vertex_t::Node`: osso usado pela posição;
- `Normal_t::Node`: osso usado pela normal.

Isso corresponde diretamente ao contrato dos shaders de referência:

```
Bones.x = bone da posição
Bones.y = bone da normal
```

## ACHADO CRÍTICO B1

O `VertexBMD` atual contém apenas:

```cpp
GLuint m_iBone;
```

Logo, o pipeline VAO atual só consegue transportar um índice de osso.

### Impacto

O formato atual perde a informação de `Normal_t::Node` caso os dois Nodes sejam diferentes.

### Correção arquitetural

O contrato correto para o cliente é:

```cpp
struct VertexBMD
{
    vec3_t m_vPos;
    vec3_t m_vNorm;
    vec2_t m_vTex;
    GLuint m_iBones[2];
};
```

Não usar 4 pesos como contrato principal, porque isso não corresponde ao BMD nem aos shaders de referência.

---

# 3. SendBonePalette — RESULTADO

## Caminho atual

`AddBoneTransform()` recebe:

```
float BoneMatrix[][3][4]
```

e `BuildModelBonePalette()` converte cada bone para 12 floats.

O resultado é:

```
3 linhas × 4 floats × NumBones
```

Depois cada comando de mesh recebe:

```
std::shared_ptr<std::vector<float>> m_BonePalette
```

No render:

```cpp
glUniform4fv(
    locations.bones,
    palette->size() / 4,
    &(*palette)[0]);
```

## ACHADO CRÍTICO B2 — cache inválido por identidade de shared_ptr

O cache atual considera a palette enviada quando:

```
palette.get() == cachedPalettePointer
```

Mas o conteúdo do `std::vector<float>` pode mudar mantendo o mesmo endereço do objeto.

### Consequência

Uma nova animação pode reutilizar o mesmo objeto `m_CurrentBonePalette`, alterar os floats e o cache ainda considerar a GPU atualizada.

Isso é exatamente o tipo de problema que a arquitetura versionada deve evitar.

### Correção

A identidade de upload deve incluir:

- versão/generation da animação; ou
- contador de frame/pose; ou
- upload explícito sem cache até existir versionamento correto.

---

# 4. ACHADO CRÍTICO B3 — palette não usa a referência real de skeleton transport

O pipeline atual usa:

```
glUniform4fv -> u_Bones
```

A referência OpenGL/Vulkan usa:

```
BonesTexture
RequestBone(BoneIndex, BoneId)
```

### Conclusão

`SendBonePalette()` é um caminho legado/intermediário.

Não deve ser estendido como solução final para GPU skinning multi-backend.

---

# 5. ACHADO CRÍTICO B4 — NumBones pode não representar a palette ativa completa

`BuildModelBonePalette()` limita:

```cpp
boneCount = min(model->NumBones, 200);
```

Para modelos que compartilham skeleton de outro objeto, `model->NumBones` pode ser diferente da quantidade de transforms acessíveis pela geometria.

A referência externa do MuMain Sven também documenta explicitamente o risco de armaduras com `NumBones` pequeno usando índices do skeleton completo. citeturn0search0turn0search1

### Ação

A futura `SkeletonBuffer` não deve inferir o tamanho da palette apenas de `model->NumBones`.

A fonte da palette ativa deve declarar explicitamente:

```
pointer
boneCount
generation/version
owner/baseOffset
```

---

# 6. ACHADO B5 — transformação de escala/origem está embutida na palette

`BuildModelBonePalette()` aplica:

- BodyScale;
- m_fRequestScale;
- BodyOrigin.

Isso cria uma palette já pré-transformada para o contexto do objeto.

### Risco

A referência dos shaders separa:

```
Bone transform
+
BodyScale
+
BodyOrigin
```

Misturar esses conceitos dentro da palette reduz reutilização e dificulta instancing.

### Decisão

Na arquitetura nova:

```
SkeletonBuffer = somente skeleton transforms
Instance/Object data = BodyOrigin + BodyScale
```

---

# 7. ACHADO B6 — BoneIndex por instância ainda não existe no frontend atual

O shader de referência recebe `BoneIndex` por instância.

O `RenderMeshVAO` atual possui:

```
m_OldBMD
m_IndexMesh
m_BonePalette
```

mas não possui:

```
SkeletonBase / BoneIndex
```

### Ação

Adicionar posteriormente um identificador de slice/base da skeleton allocation ao comando de renderização.

---

# 8. ACHADO B7 — infraestrutura BoneUBO está parcialmente desconectada

`New_ModelBMD.h` inclui:

```
Render/Model/BMDGpuSkinningUBOBinding.h
```

e declara:

```
Render::BMDGpuSkinningUBOBinding m_BoneUBO;
bool UploadBonePaletteUBO(...);
```

A busca na branch não encontrou implementação de `UploadBonePaletteUBO`.

### Conclusão

A infraestrutura existe no header, mas não está integrada ao caminho de renderização auditado.

### Ação

Não expandir essa rota até decidir se será removida, mantida como experimento ou transformada em backend alternativo.

---

# 9. CONTRATO CORRETO EXTRAÍDO

## Dados estáticos do mesh

```
Position
UV
Normal
PositionBone
NormalBone
```

## Dados dinâmicos por instância

```
SkeletonBase/BoneIndex
BodyOrigin
BodyScale
BodyLight
...```

## Dados dinâmicos de skeleton

```
SkeletonBuffer
  [bone matrix or dual quaternion]
```

---

# 10. PRÓXIMA IMPLEMENTAÇÃO RECOMENDADA

## Passo 1 — corrigir somente o contrato CPU/Vertex

Atualizar `VertexBMD` para armazenar dois bones, sem ativar ainda GPU skinning.

## Passo 2 — auditar ExtendVertex

Garantir:

```
m_iBones[0] = Vertices[v].Node
m_iBones[1] = Normals[n].Node
```

## Passo 3 — VAO

Definir location 3 como:

```
GL_UNSIGNED_INT × 2
```

compatível com `uvec2 Bones`.

## Passo 4 — não alterar shader runtime antigo ainda

O shader atual `Model.vs` continua sendo parte do pipeline existente.

Primeiro criar/adaptar o caminho que realmente consome o contrato dos shaders OpenGL/Vulkan de referência.

## Passo 5 — SkeletonBuffer

Criar frontend compartilhado com:

```
Upload(pointer, count, generation)
AllocateSlice()
GetBaseIndex()
Bind(backend)
```

---

# VEREDITO

## Confirmado

- `Vertex_t::Node` e `Normal_t::Node` explicam exatamente o `uvec2 Bones` da referência.
- O BMD original é naturalmente compatível com o contrato OpenGL/Vulkan estudado.
- O pipeline atual perdeu/ignora parte dessa semântica ao reduzir o vertex para um único `m_iBone`.

## Não fazer

- Não promover 4 pesos/4 bones.
- Não consolidar BoneUBO como contrato definitivo.
- Não remover CPU fallback.

## Fazer agora

1. corrigir o formato `VertexBMD` para dois bones;
2. auditar e corrigir `ExtendVertex`;
3. preparar VAO location 3 = uvec2;
4. depois implementar SkeletonBuffer/BonesTexture.
