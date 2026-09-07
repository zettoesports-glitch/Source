# SHADER_REPOS_SURVEY.md
**Fase 1 — Levantamento dos repositórios de shaders**
**Data:** 2026-09-06
**Localização (confirmada pelo dono):** `C:\MUVULKAN\Shaders e winrar`

> Os dois repos estão duplo-aninhados: `opengl-main/opengl-main/OpenGL` e
> `vulkan-main/vulkan-main/Shaders`.

---

## 1. `opengl-main/opengl-main/OpenGL` — shaders GLSL (decompilados)

**Arquivos:** 176 `.vs` (vertex) + 64 `.ps` (pixel/fragment).

**Categorias:** `Shadow0/`, `Shadow1/`, `Shadow2/`, `cloth/`, `effects/`,
`fonts/`, `interface/`, `models/`, `shadows/`, `shadowscloth/`, `terrains/`.

**Formato:** GLSL `#version 330` estilo **SPIRV-Cross** — cabeçalho
`// Generated from models/chrome01.vs. Edit the HLSL source and regenerate this file.`
Ou seja: são artefatos cross-compilados (HLSL → SPIR-V → GLSL), legíveis mas
não-fonte. O `_entryPointOutput` e structs geradas confirmam SPIRV-Cross.

**Achados importantes (em `models/`):**
- Inputs de **GPU skinning** já presentes: `uvec2 Bones`, `uint BoneIndex`,
  `BodyOrigin/BodyScale/BodyLight/ShadowLight`.
- `layout(std140) uniform GlobalConstants` — ShadowColor, ShadowPosition,
  matriz Shadow, **Game3D, Game2D** (matrizes 3D/2D), BillboardQ, LightPosition,
  WorldTime → **este é o layout de referência para o GlobalUBO do plano (seção 8)**.

---

## 2. `vulkan-main/vulkan-main/Shaders` — HLSL (fonte legível)

**Arquivos:** 45 `.vs` + 17 `.ps` + 8 `.inc`.

**Categorias:** `cloth/`, `common/`, `containers/`, `effects/`, `fonts/`,
`interface/`, `models/`, `shadows/`, `shadowscloth/`, `terrains/`.

**Formato:** **HLSL real** (não decompilado): `cbuffer ... : register(b0)`,
`mul()`, `#include`. É a fonte de referência; o repo OpenGL é o mesmo material
em GLSL decompilado.

**Includes em `common/` (os mais valiosos):**
| Include | Conteúdo |
|---|---|
| `GlobalConstantBuffers.inc` | `GlobalConstants` em `register(b0)` — layout do GlobalUBO |
| `SkeletonHelpers.inc` | **helpers de skinning** (ver §3) |
| `VertexDefinitions.inc` / `PixelDefinitions.inc` | structs de entrada/saída |
| `TerrainConstantBuffers.inc` / `TerrainHelpers.inc` / `TerrainModelHelpers.inc` | terreno |
| `ShadowCHS.inc` | shadow (compute/hull?) |

**Achado importante — `SkeletonHelpers.inc`:**
- Osso vem por **textura**, não UBO: `BonesTexture` (`Buffer<float4>` ou
  `Texture2D`, `register(t0)`), endereçado por `SKELETON_TEXTURE_WIDTH`
  (definido no motor C++, não nos shaders).
- `RequestBone(boneIndex, boneId)` busca a matriz do osso; skinning de 2 ossos
  por vértice via `Bones.x`/`Bones.y`.
- Suporta **dual quaternion skinning** (`USE_DUAL_QUATERNION`) além de matriz
  padrão, via `#if`.
- Valida a seção 16 do plano ("BoneUBO / **Texture Buffer**"): a referência
  escolheu textura — provavelmente por caber qualquer quantidade de ossos
  sem limite tipo MAX_BONES=200 no UBO.

---

## 3. Implicações para o plano

1. **FASE 7 (GPU Skinning):** `SkeletonHelpers.inc` + shaders de `models/`
   são a implementação de referência — estudar antes de escrever shader novo.
   O formato de input (`uvec2 Bones`, `uint BoneIndex`) define como o
   BoneManager/New_ModelBMD deve alimentar a GPU.
2. **FASE 4 (GlobalUBO):** adotar `GlobalConstants` como layout inicial do
   GlobalUBO (já cobre shadow, Game3D/Game2D, luz, tempo, billboard).
3. **HLSL como fonte:** manter os `.inc` do repo Vulkan como fonte da verdade
   e regenerar GLSL quando necessário (o repositório OpenGL é derivado).
4. **MAX_BONES vs textura de ossos:** o plano fixa MAX_BONES=200 para UBO; a
   referência usa textura de ossos sem limite fixo — decidir na FASE 7
   (textura é mais flexível e é o caminho que os shaders esperam).
5. `SKELETON_TEXTURE_WIDTH` e os `#define USE_*` vêm do motor (C++) — quando
   o MuMain Sven estiver disponível, localizar esses defines lá.
