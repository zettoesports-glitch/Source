# RendererModel — reconstrução profunda

## Evidência forte

- vertex layout: position, normal, texcoord, bone;
- `u_Bones[200]` e decode `bone/3`;
- FrameData UBO na variante moderna;
- Chrome modes 1..10, `u_BlendMeshTexCoord`, fog e alpha cutoff;
- shader de shadow com `u_BodyOrigin`, `u_SunOffset`, `u_ShadowStrength`;
- `full_pose_ubo_selected=0` / `measured_sparse_mesh_bone_sets`;
- fases G01, G02, G03, G04, G06, G07, H08.

## Pipeline reconstruído

1. Captura matrizes/frame em FrameData.
2. Converte estado fixed-function antigo para `ModelMaterialState` explícito.
3. Monta palette/scratch apenas dos bones usados pela mesh.
4. Reaproveita pose por handle quando possível.
5. Enfileira model/shadow commands.
6. Transparência vira barreira.
7. Apenas runs opacos com mesma pose/programa/geometria podem ser reordenados.
8. Commit de estado/uniforms elimina chamadas repetidas.

## Regra G07 literal

`contiguous_opaque_same_pose_program_geometry`; também aparecem `cross_pose_sort=0`, `cross_program_sort=0`, `cross_geometry_sort=0`, `stable_sequence_tiebreak=1` e barreiras transparentes. O `ModelCommandQueue` desta pasta implementa essa política clean-room.
