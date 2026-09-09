# 36 — RenderManager / command ordering exato

Status: **CONFIRMADO POR SOURCE + BINÁRIO**.

## Command manager

`NRenderManager` mantém um vetor de `RCommandList` e um `DraftCommandList` atual.

Capacidade inicial e crescimento:

```text
4096 command lists por incremento
```

Antes da execução, se houver mais de uma command list, executa:

```cpp
std::stable_sort(... lhs.Id < rhs.Id)
```

## 11 tipos de comando exatos

```text
UpdateBuffer
UpdateBufferWithMap
UpdateTexture
SetDynamicTexture
SetDynamicBuffer
SetPipelineState
SetVertexBuffer
SetIndexBuffer
CommitShaderResources
Draw
DrawIndexed
```

## Transições

Há duas camadas:

1. `StateTransitions` globais executadas antes das command lists;
2. transições acumuladas durante uma command list, aplicadas imediatamente antes de `Draw` ou `DrawIndexed`.

`CommitShaderResources` usa:

```text
TRANSITION  se ShaderResourceBinding.ShouldTransition == true
VERIFY      como fallback quando não foi pedido NONE
```

e depois limpa `ShouldTransition`.

## Ordenação classificada

Hash de 64 bits:

```text
bits 63..56  View (8 bits)
bits 55..53  NDrawOrderType (3 bits)
bits 52..51  RenderClassify (2 bits)
bits 50..43  Index (8 bits)
bits 42..27  Shader (16 bits)
```

O `RenderClassify` pode vir explicitamente do draw ou ser derivado do estado atual do pipeline (DepthWrite, BlendEnable, SrcBlend, DestBlend, BlendHash).

## Ordenação sequencial

Hash:

```text
View (8 bits)
NDrawOrderType (3 bits)
Index (32 bits)
```

Usado quando a ordem explícita precisa ser preservada.

## Consequência

O sistema não é um simples sort por material. Ele possui dois contratos:

```text
Classifier → permite reordenar por classe/shader para reduzir estados
Sequential → preserva a sequência solicitada
```

O `stable_sort` mantém a ordem original quando as chaves são iguais.

## Recomendação para nosso MU

Implementar no frontend uma fila equivalente:

```text
RenderPacket
  ├─ order mode: Classified | Sequential
  ├─ view
  ├─ render class
  ├─ shader/pipeline key
  ├─ sequence/index
  └─ commands/resources
```

Isso permite compartilhar batching/ordering entre OpenGL 4.6+ e Vulkan sem quebrar alpha/transparência.
