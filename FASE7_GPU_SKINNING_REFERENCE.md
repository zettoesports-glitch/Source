# FASE 7 — Referência de GPU Skinning OpenGL/Vulkan

## Fonte de verdade

A implementação de referência do repositório é:

```
vulkan-main/vulkan-main/Shaders/
```

Os shaders Vulkan estão em HLSL fonte. Os shaders OpenGL em:

```
opengl-main/opengl-main/OpenGL/
```

são artefatos GLSL gerados/cross-compilados da mesma lógica.

Portanto, alterações de comportamento de skinning devem nascer na arquitetura compartilhada/HLSL e somente depois refletir no backend OpenGL.

## Contrato real encontrado

```
VertexInput
location 0  Position
location 1  UV
location 2  Normal
location 3  uvec2 Bones
location 12 uint BoneIndex (instância)
```

O BMD atual possui um Node por vértice. A referência usa dois índices de bones, permitindo:

- Bones.x == Bones.y: um único bone efetivo
- Bones.x != Bones.y: posição e normal podem usar bones distintos conforme shader/modelo

## Transporte de skeleton

A referência não usa BoneUBO fixo como caminho principal.

```
CPU Bone Palette
      ↓
Skeleton Texture / Texture Buffer
      ↓
BonesTexture
      ↓
RequestBone(BoneIndex, BoneId)
      ↓
Vertex Shader
```

Cada matriz padrão ocupa quatro texels `float4`.

O helper `SkeletonHelpers.inc` também suporta Dual Quaternion via `USE_DUAL_QUATERNION`.

## Regra de backend

```
Shared model/skeleton data
          ↓
     Render frontend
      /          \
 OpenGL backend   Vulkan backend
      |                |
 GLSL runtime       HLSL/SPIR-V
```

A lógica do modelo e os dados de skeleton devem permanecer compartilhados. A criação e binding físico do recurso são responsabilidade do backend.

## Decisão da FASE 7

Não ativar globalmente o BoneUBO de 200 bones criado durante a infraestrutura até comparar com a referência real.

O caminho preferencial passa a ser:

1. manter BonePalette compartilhada;
2. criar uma abstração de SkeletonBuffer;
3. implementar backend OpenGL como texture buffer/texture;
4. preparar backend Vulkan com o mesmo contrato de dados;
5. manter CPU skinning como fallback;
6. validar visualmente antes de promover o caminho GPU.

## Próxima integração física

- mapear `Vertex_t::Node` para o formato real `uvec2 Bones`;
- localizar o upload de skeleton/bones no cliente;
- criar `SkeletonBuffer` compartilhado;
- implementar OpenGL primeiro;
- manter a interface preparada para Vulkan futuro;
- somente então adaptar o shader runtime correspondente.
