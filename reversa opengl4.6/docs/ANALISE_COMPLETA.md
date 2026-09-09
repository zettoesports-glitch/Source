# Análise estática completa — Main.exe

## Escopo

Análise realizada sem executar o binário. Foram inspecionados cabeçalhos PE, seções, imports, CodeView/PDB, strings, shaders GLSL e trechos de assembly x86.

## PE

| Campo | Valor |
|---|---|
| Arquitetura | Intel i386 / PE32 |
| Subsystem | Windows GUI |
| ImageBase | 0x00400000 |
| Entry point | RVA 0x006FBCC3 / VA 0x00AFBCC3 |
| Timestamp | 2026-09-08 09:09:32 |
| Relocations | stripped / sem `.reloc` |
| NX | compatível |
| Seções | `.text`, `.rdata`, `.data`, `.fptable`, `.rsrc` |

### Seções

- `.text`: VMA `0x00401000`, raw offset `0x00000400`, size `0x00781AED`
- `.rdata`: VMA `0x00B83000`, raw offset `0x00782000`, size `0x000E8F6E`
- `.data`: VMA `0x00C6C000`, raw offset `0x0086B000`, size `0x00058800`
- `.fptable`: VMA `0x09AEE000`, raw offset `0x008C3800`, size `0x80`
- `.rsrc`: VMA `0x09AEF000`, raw offset `0x008C3A00`, size `0x220A0`

## Debug/PDB

CodeView RSDS presente, age 7, PDB:

`C:\Genesys\x6 Plus\Src S6 Plus\SRC\Main5.2\Release_EX603\Main.pdb`

Isso, junto com RTTI/strings/shaders legíveis, é incompatível com a hipótese de um packer pesado envolvendo toda a imagem.

## OpenGL / seleção do renderer

A string `OpenGlUseShader` está em file offset `0x007D61D0` / VA `0x00BD71D0` e é referenciada pela rotina de configuração. A chave de registro é `SOFTWARE\Webzen\Mu2\Config`.

Strings literais confirmam:

- `OpenGL 2 (legacy)`
- `OpenGL 3.3 Compatibility`
- `OpenGL 4.6`
- `Renderer registry value %d is invalid; defaulting to OpenGL 3.3`

### Criação de contexto

Função reconstruída em torno de VA `0x00470C30`:

- resolve `wglCreateContextAttribsARB` por `wglGetProcAddress`;
- modo 2 tenta major/minor 4.6;
- perfil 4.6 é calculado como Core (`1`) ou Compatibility (`2`);
- caminho de fallback usa 3.3 Compatibility (`profile mask = 2`);
- helper em `0x00470D90` monta atributos WGL `0x2091` (major), `0x2092` (minor), `0x2094` (flags), `0x9126` (profile mask), terminador 0;
- em falha registra `GetLastError`, apaga contexto candidato e tenta restaurar o contexto anterior.

## Capabilities GL4 observadas

O binário registra explicitamente:

- `cap_profile_compatibility`
- `cap_profile_core`
- `cap_gl33_bridge`
- `cap_gl46_bridge`
- `cap_shader_objects`
- `cap_buffer_objects`
- `cap_vertex_array_objects`
- `cap_texture_storage`
- `cap_timer_query`
- `cap_debug_output`
- `cap_direct_state_access`
- `cap_buffer_storage`
- `cap_uniform_buffer`
- `cap_shader_storage_buffer`
- `cap_sampler_objects`
- `cap_sync_objects`
- `cap_instanced_drawing`
- `cap_multi_draw_indirect`
- `cap_compute_shader`

Também registra limites de textura, texture units, vertex attribs, UBO/SSBO bindings, MSAA, array layers e tamanhos máximos de UBO/SSBO.

## RendererModel

Shaders literais recuperados confirmam dois vertex paths:

- versão com `uProj/uView` explícitos;
- versão com `FrameData` UBO `std140`.

Layout do vértice observado:

```glsl
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTex;
layout(location = 3) in uint aBone;
```

Skinning:

```glsl
int idx = int(aBone) / 3;
mat4 bone = u_Bones[idx];
vec4 worldPos = bone * vec4(aPos, 1.0);
```

`u_Bones` possui 200 matrizes. O shader de material contém Chrome modes, `u_BlendMeshTexCoord`, fog e alpha cutoff. O shadow vertex shader usa `u_BodyOrigin` + `u_SunOffset` para projetar a geometria no plano.

### Etapas/otimizações nomeadas

- `G01ModelExplicitMatrices`
- `G02ModelMaterialState`
- `G03ModelBoneScratch`
- `G04ModelPoseReuse`
- `G06ModelQueueObserve`
- `G07ModelOpaqueSort`
- `H08ModelStateCommit`
- `H08ModelInstanceUniformCache`

A string `full_pose_ubo_selected=0` com `full_pose_ubo_rejection_reason=measured_sparse_mesh_bone_sets` confirma que uma estratégia de pose inteira por UBO foi considerada/rejeitada para esse caminho.

## Outros renderers observados

- `Renderer2DGL46`
- `RendererSkyGL46`
- `RendererAtmosphereGL46`
- `RendererWaterGL46`
- `RendererClothGL46`
- `RendererTerrainGL46`

`Renderer2DGL46` registra draws específicos de chat, Lua color box, ordered batch, solid bitmap, rotated bitmap, sprites, numbers, effect sprites, particle billboards e text surfaces, além de cache de texto e batch de HP bar.

Terrain contém nomes de uniform/estado de clima/água como `waterLevel`, `waterDepthRange`, `waterTurbidity`, `waterFrost`, `waterLava`, `footstepCount`, `footsteps[0]` e métricas P01 para redução/telemetria de state changes.

## Shaders recuperados

Foram encontrados 33 strings iniciadas em `#version`. Nem todas são do renderer do jogo: há conteúdo de ImGui e RmlUi. O manifesto em `shaders/SHADER_MANIFEST.txt` mantém offset e tamanho de cada bloco para auditoria.

Os shaders 7–13 são diretamente associados ao `RendererModel.cpp` pelo posicionamento em `.rdata` e conteúdo. Shaders 14–21 são passes GL 3.3 adicionais presentes na mesma região, mas a associação exata a classes específicas ainda deve ser tratada como não confirmada.

## Paths de source recuperados

Veja `evidence/source_paths.txt`. Entre os caminhos confirmados estão `RenderDevice3D.cpp`, `RendererModel.cpp`, `GLCompatFixedState.cpp`, `MapAtmosphere.cpp`, `ZzzBMD.cpp`, `ZzzOpenglUtil.cpp`, `Sprite.cpp`, `ZzzEffectParticle.cpp`, `ZzzEffectJoint.cpp`, `ZzzCharacter.cpp`, `Winmain.cpp` e outros.

## O que ainda não é possível chamar de source original

- nomes das funções que não aparecem em RTTI/PDB público;
- nomes de variáveis locais;
- headers/tipos exatos;
- templates/macros originais;
- comentários que não sobreviveram à compilação;
- ordem/estrutura de translation units que não deixaram paths/strings;
- lógica inteira do jogo, que exigiria decompilar milhares de funções.

A pasta `source_reconstructed` fornece uma reconstrução limpa do que já está confirmado, sem afirmar equivalência textual com a source original.
