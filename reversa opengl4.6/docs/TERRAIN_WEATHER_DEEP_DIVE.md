# Terrain / clima / água

O fragment shader de terrain foi recuperado em fragmentos legíveis e está em `shaders/extracted_gl46/Terrain_E07_E08_weather.frag.fragments.glsl`.

Uniforms confirmados incluem `terrainSurface`, `weatherEnabled`, `waterFxEnabled`, `rainSurfaceFxEnabled`, `wetGroundFxEnabled`, `footstepFxEnabled`, `weatherQuality`, `weatherTime`, `rainIntensity`, `waterLevel`, `waterDepthRange`, `waterTurbidity`, `waterFrost`, `waterLava`, `footstepCount` e `footsteps[16]` (8 passos x 2 vec3).

A água usa sete ondas em qualidade alta; há rain ripples, Fresnel, profundidade, turbidez, gelo e lava. O wet-ground path combina chuva, marcas de pegadas e specular. E07/E08 usam command encoder/batches e P01 mede deltas de depth/cull/depth-mask/blend/texture/alpha/fog e tempos de replay/loop.
