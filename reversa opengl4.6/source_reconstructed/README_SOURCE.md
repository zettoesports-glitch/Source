# Source reconstruída OpenGL 4.6

Implementação **clean-room de referência**, não source original. Arquivos são classificados como EXTRACTED (literal), RECONSTRUCTED (fluxo derivado de assembly/xrefs) ou INFERRED (API limpa criada para representar a evidência).

## Núcleo
- RendererSelection / RenderDevice3D: GL2 → GL3.3 Compatibility → GL4.6/fallback.
- GL46Capabilities: capability matrix observada.
- GLCorePrimitiveStream: referência para substituir primitive/immediate-mode.
- LegacyReachabilityAudit: auditoria do fixed-function.

## Model
- RendererModel + ModelCommandQueue: FrameData, 200 bones, material, pose, queue e G07 sorting conservador.

## 2D
- Renderer2DGL46 + Renderer2DBatcher + TextSurfaceCache.

## Terrain/environment
- RendererTerrainGL46 + TerrainCommandEncoder + Water/Sky/Atmosphere.

## Effects
- RendererClothGL46 + EffectsAdjacentBatcher.

Os shaders literais/reconstruídos ficam fora da biblioteca em `../shaders/` para manter clara a proveniência.

Esta árvore foi compilada localmente como biblioteca C++17 para validar consistência interna; isso não transforma código inferido em source original do executável.
