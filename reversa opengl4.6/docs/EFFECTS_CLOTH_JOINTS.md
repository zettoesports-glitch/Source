# Cloth / efeitos / joints

Fases extraídas: F01ClothMesh, F02SideHairRibbon, F03CircleEffects, F05BlurRibbons, F06JointRibbons, F07UtilityPlane. Joints possuem P01JointDirectBufferUpload, SubmitTimingTelemetry, AdjacencyTelemetry e AdjacentBatch.

Métricas confirmam contagem de passes, vertices/indices/quads, draw calls, fallback legado, upload DSA/bound-buffer, timing por etapa, pares adjacentes, runs mergeáveis, flush reasons e `joint_adjacent_batch_saved_draw_calls`.

A regra clean-room implementada em `EffectsBatcher` só agrega passes adjacentes com a mesma assinatura texture/pipeline/frame e respeita barreiras.
