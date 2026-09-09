# Roadmap interno da modernização OpenGL

Os nomes abaixo são **strings literais extraídas** do `Main.exe`. A interpretação por grupos é reconstruída a partir de nomes, métricas e xrefs.

## D — 2D/UI

- `D01ChatFade`
- `D02LuaColorBox`
- `D03OrderedBatch`
- `D04SolidBitmapHelpers`
- `D05RotatedBitmapHelpers`
- `D06GenericSprite`
- `D07NumbersAndPoints`
- `D08EffectSprites`
- `D09ParticleBillboards`
- `D11TextSurfaceCache`

## E — Ambiente/terrain/water

- `E01SkyCylinder`
- `E02AtmosphereHaze`
- `E05WaterTile`
- `E06HellasWater`
- `E07TerrainCommandEncoder`
- `E08TerrainBatches`

## F — Cloth/effects

- `F01ClothMesh`
- `F02SideHairRibbon`
- `F03CircleEffects`
- `F05BlurRibbons`
- `F06JointRibbons`
- `F07UtilityPlane`

## G — Model/BMD

- `G01ModelExplicitMatrices`
- `G02ModelMaterialState`
- `G03ModelBoneScratch`
- `G04ModelPoseReuse`
- `G06ModelQueueObserve`
- `G07ModelOpaqueSort`

## H — Compatibilidade/remoção do legado

- `H04CompatibilityFixedStateBypass`
- `H04CpuModelTriangles`
- `H05CoreProfile`
- `H08ModelInstanceUniformCache`
- `H08ModelStateCommit`

## P — Otimizações/telemetria

- `P01HeadNameTextReuse`
- `P01JointAdjacencyTelemetry`
- `P01JointAdjacentBatch`
- `P01JointDirectBufferUpload`
- `P01JointSubmitTimingTelemetry`
- `P01MonsterHpBarBatch`
- `P01ParticleDeferredTextureBind`
- `P01ParticleDirectZRotation`
- `P01TerrainLoopSampleTelemetry`
- `P01TerrainReplayTimingTelemetry`
- `P01TerrainStateDelta`
- `P01TerrainTransitionTelemetry`
- `P02FootstepFX`
- `P02RainPreviewIntensity`
- `P02RainSurfaceFX`
- `P02TerrainWeatherFX`
- `P02TerrainWeatherQuality`
- `P02WaterFX`
- `P02WetGroundFX`
