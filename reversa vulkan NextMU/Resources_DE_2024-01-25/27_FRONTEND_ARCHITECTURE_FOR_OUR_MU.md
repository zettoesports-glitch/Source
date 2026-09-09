# 27 — Frontend gráfico recomendado para nosso MU

Status: **RECONSTRUÍDO** a partir das arquiteturas confirmadas do Main OpenGL 4.6 e do NextMU/Diligent/Vulkan.

Não é source original do NextMU. É a arquitetura proposta para aproveitar as melhores ideias encontradas em ambos os clientes.

## Objetivo

Escrever lógica de Model/Terrain/UI/Effects uma vez e permitir backends:

```text
OpenGL 4.6+
Vulkan
```

sem duplicar gameplay/render rules.

## Camadas

```text
Game / MU objects
      ↓
Render Extraction
      ↓
ModelDrawPacket / TerrainPacket / QuadPacket / UI Packet
      ↓
RenderStateKey + ResourceBindings
      ↓
DynamicUniformArena / GeometryStream
      ↓
RenderCommandBuffer
      ↓
Pass-aware batching / safe sorting
      ↓
Backend interface
      ├── OpenGL46Backend
      └── VulkanBackend
```

## 1. Render packets

Nenhuma função de BMD/NPC/Player deve chamar diretamente `gl*` ou `vk*`.

Exemplo:

```cpp
struct ModelDrawPacket
{
    ModelHandle model;
    MeshHandle mesh;
    MaterialHandle material;
    PoseHandle pose;
    Transform transform;
    RenderStateKey state;
};
```

## 2. RenderStateKey data-driven

Usar a ideia do `model.json`:

```text
depth
cull
blend
blend equation
alpha test
program/shader variant
premultiply flags
```

O mesmo estado é traduzido pelo backend:

```text
RenderStateKey
  ├─ GL: glDepthFunc/glBlendFunc/glCullFace/cache
  └─ VK: VkGraphicsPipeline / PSO cache key
```

## 3. Uniform arena

Adotar páginas reutilizáveis como o NextMU:

```text
ModelViewProj : 128-byte slices
ModelSettings : 96-byte slices (ou nossa struct final após semantic mapping)
```

No GL:
- UBO ranges / buffer storage / DSA.

No Vulkan:
- dynamic uniform/storage offsets, ring/frame arenas.

## 4. QuadStream compartilhado

Particles e Joints provam a mesma geometria física:

```text
4 vertices × 36 B
6 uint32 indices
```

Criar uma stream comum para:
- particles;
- joints;
- ribbons/sprites quando layout compatível;
- UI em stream separada caso o vertex format seja diferente.

## 5. Command buffer frontend

Inspirado diretamente no NextMU:

```text
UpdateBuffer
Map/Upload
UpdateTexture
BindResource
SetPipelineState
SetVertexBuffers
SetIndexBuffer
CommitResources
Draw
DrawIndexed
```

O backend executa depois e agrupa transitions/state changes.

## 6. Pass order

Preservar ordem confirmada como ponto de partida:

```text
Terrain
Objects
Characters
Particles
Joints
UI
```

Sorting apenas dentro de domínios seguros. Transparência cria barreiras.

## 7. Model shared path

Hero, remote player, BotBuffer, NPC, Monster e world objects devem convergir para um único Model renderer, diferenciados pelos dados do packet, nunca por globals de estado implícito.

Isso ataca diretamente o problema que já vimos no nosso ModernBMD de state leakage entre instâncias.

## 8. Backend API mínima

```cpp
class IRenderBackend {
public:
    virtual void BeginFrame() = 0;
    virtual void Execute(const RenderCommandBuffer&) = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
};
```

Recursos devem ser handles estáveis do frontend; objetos GL/Vulkan ficam encapsulados no backend.

## 9. Ordem de implementação recomendada

```text
1 Context/device + debug
2 RenderStateKey + cache
3 shader/resource abstraction
4 DynamicUniformArena
5 shared Model renderer
6 Hero + remote/Bot + NPC/Monster
7 command buffer
8 Particle/Joint QuadStream
9 Terrain
10 2D/UI
11 safe batching/sorting
12 GPU telemetry
13 Vulkan backend paralelo
14 GL4.6+/Vulkan advanced features somente com métricas
```

## 10. Regra de estabilidade

Nenhuma feature moderna deve entrar só porque a API suporta. Adotar apenas quando:
- resolve um gargalo medido;
- tem fallback;
- mantém equivalência visual;
- passa por validação/debug;
- não cria dependência de estado global por objeto.
