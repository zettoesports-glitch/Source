# Arquitetura OpenGL 4.6 reconstruída

## Bootstrap

`OpenGlUseShader` possui contrato literal `0,1,2`: GL2 legado, GL3.3 Compatibility e GL4.6. A rotina em `0x00470C30` resolve `wglCreateContextAttribsARB`; o helper em `0x00470D90` monta major/minor/flags/profile. GL4.6 pode pedir Core ou Compatibility; em falha tenta 3.3 Compatibility e por fim conserva o contexto legado.

## Compatibilidade dos shaders

O contexto 4.6 não implica GLSL 460 em todos os programas. O binário preserva muitos shaders em GLSL 330 core para compartilhamento GL3.3/GL4.6, enquanto o backend 4.6 aproveita APIs adicionais (DSA, buffer storage, debug output etc. quando disponíveis).

## FrameData

Vários programas usam UBO `std140, binding=0`: view, projection, viewProjection, cameraPosition, viewport, timeFog e tint.

## Model/BMD

`RendererModel` usa vértice position/normal/uv/bone e `u_Bones[200]`; o bone index observado é `encoded/3`. Material moderno cobre texture/default/bright/chrome/shadow, BlendMesh, alpha cutoff e fog. O roadmap G01..G07/H08 evidencia matrizes explícitas, estado de material, scratch de bones, pose reuse, command queue, ordenação opaca conservadora e cache de uniforms.

## 2D/UI

D01..D14 cobrem chat fade, Lua color box, ordered batch, bitmap helpers, sprites, numbers, effects, particles, text surface/cache, ImGui, RmlUi e UI3D matrix scope. P01 adiciona batch de HP bar, reuse de nome e otimizações de partículas.

## Ambiente

E01 = sky cylinder; E02 = atmosphere haze; E05/E06 = water; E07/E08 = terrain encoder/batches. P02 injeta weather, water, rain surface, wet ground e footsteps.

## Effects/cloth

F01/F02/F03/F05/F06/F07 cobrem cloth, side hair, circles, blur, joints e utility plane. P01 de joints registra direct buffer upload, timing, adjacency e adjacent batching.

## Remoção do fixed-function

H04/H05 e a telemetria de reachability mostram um esforço explícito de impedir immediate mode, matrix stack, fog fixo, texture env, client arrays, attribute stack, GL_QUADS e current-color no caminho Core.
