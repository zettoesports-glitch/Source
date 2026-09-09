# 30 — Baseline pública do NextMU correspondente à build de 25/01/2024

Status: **CONFIRMADO FORTE — source↔binary**.

## Repositório público encontrado

```text
https://github.com/kuncarous/nextmu
```

Commit histórico usado como baseline:

```text
b0388a6416379c03dc9089d74342180eb2560042
2024-01-25T01:55:35Z
```

Parent imediato:

```text
46cb679a8a1372e9c25deb78846b56e2d851b259
2024-01-24T15:45:21Z
added rmlui full support
```

## Por que esse commit é relevante

O pacote analisado chama-se `Resources_DE_2024-01-25.zip`. O `NextMU.exe` x64 e a source histórica apresentam as mesmas características observadas por disassembly:

- seleção de backends D3D11 → Vulkan → D3D12 → OpenGL;
- nomes `GraphicsEngineVk_32r.dll` / `GraphicsEngineVk_64r.dll`;
- `GetEngineFactoryVk`;
- adapter enumeration;
- validation/debug configurável;
- warning Vulkan suprimido `UNASSIGNED-CoreValidation-Shader-OutputNotConsumed`;
- `CreateDeviceAndContextsVk`;
- `CreateSwapChainVk`;
- Diligent API version 254000;
- mesmo registry de shaders/resources/input layouts;
- mesma arquitetura de render command buffer.

## Regra da engenharia daqui em diante

A source pública não é tratada automaticamente como a build exata. Cada detalhe importante recebe um dos estados:

```text
CONFIRMADO      = source histórica e binário concordam ou há prova direta no binário
RECONSTRUÍDO    = implementação clean-room baseada em comportamento confirmado
INFERIDO        = hipótese ainda não validada
```

A baseline pública permite recuperar nomes/estruturas que o compilador removeu, enquanto o EXE determina o que efetivamente entrou nessa build.

## Fontes de maior valor

```text
client/game/src/mu_graphics.cpp
client/game/src/mu_modelrenderer.cpp
client/game/src/mu_terrain.cpp
client/game/src/mu_environment_particles.cpp
client/game/src/mu_environment_joints.cpp
client/game/include/t_graphics_layouts.h
client/game/include/t_graphics_pipelinestate.h
client/game/src/t_graphics_pipelines.cpp
client/game/include/t_graphics_shaderresources.h
client/game/include/t_particle_render.h
client/game/include/t_joint_render.h
```

## Consequência para nosso MU

Não precisamos copiar o NextMU. O valor é recuperar contratos e arquitetura suficientemente exatos para construir nosso próprio frontend:

```text
MU gameplay
   ↓
Render packets
   ↓
PSO/SRB/resource cache
   ↓
Render command buffer
   ↓
OpenGL 4.6+ / Vulkan
```
