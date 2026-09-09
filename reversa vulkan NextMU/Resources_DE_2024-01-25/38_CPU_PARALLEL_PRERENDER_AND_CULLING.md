# 38 — CPU parallel PreRender, culling e skeleton upload

Status: **SOURCE-CORRELATED** com `mu_environment.cpp`, `mu_environment_characters.cpp` e `mu_environment_objects.cpp` históricos.

## Modelo de threading correto

A build usa:

```text
CPU: multithreaded update/pre-render
GPU: one immediate Diligent context + custom RenderManager replay
```

Isto corrige a interpretação simplificada de que o renderer inteiro seria single-thread.

`NEnvironment` define uma arquitetura multi e Characters/Objects chamam `MUThreadsManager::Run()` com executores sobre views EnTT.

## Characters — PreRender paralelo

A view inclui, entre outros:

```text
NRenderable
NAttachment
NLight
NRenderState
NSkeleton
NPosition
NAnimation
NBoundingBoxes
```

Para cada character em worker thread:

```text
atualiza animação
configura parent skeleton
calcula OBB/AABB
inclui parts/attachments no bbox
main-camera frustum culling
shadow-cascade frustum culling
calcula light
anima skeleton somente se necessário
Upload() skeleton -> SkeletonOffset
processa skeletons de parts linkadas
```

Só depois, no `Render()` sequencial, são emitidos Model draw packets.

## Visibility

Main pass:

```text
Diligent::GetBoxVisibility(camera frustum, AABB)
```

Shadow:

```text
loop cascades
GetBoxVisibility(cascade frustum, AABB, OPEN_NEAR)
primeira cascade visível -> renderState.ShadowVisible
```

Se não estiver visível nem na câmera nem em shadow:

```text
return antes de skeleton upload/render prep pesado
```

## Skeleton offsets

Cada skeleton visível chama:

```text
SkeletonOffset = Skeleton.Instance.Upload()
```

Parts com skeleton próprio/linkado recebem offsets independentes na mesma skeleton texture global.

## Objects

Objects seguem praticamente a mesma estrutura:

```text
parallel bbox/skeleton/light/culling
+ fading tests
+ skeleton upload
```

Existe ainda atualização paralela de grupos de fading em uma segunda `MUThreadsManager::Run()`.

## Separação frontend/backend

Arquitetura observada:

```text
worker threads
   -> animation/culling/light/skeleton staging

main render phase
   -> MUModelRenderer packets
   -> RenderManager command lists
   -> stable sort
   -> immediate DeviceContext replay
```

## Recomendação para nosso MU

Não começar Vulkan com vários command buffers GPU por thread.

Primeiro reproduzir a estratégia mais simples e estável:

```text
parallel CPU visibility/animation/packet building
single backend submission/replay
```

Depois de profiling real, podemos testar Vulkan secondary command buffers ou parallel command recording como evolução separada.
