# 35 — Render sorting / command order exato

Status: **SOURCE-CORRELATED** com `t_graphics_rendermanager.h/.cpp` e `t_graphics_renderclassifier.*` históricos, consistente com a command queue observada no EXE.

## 11 comandos

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

## Command lists

Cada draw fecha a command list atual e abre uma nova. O manager cresce em blocos de:

```text
4096 command lists
```

Antes de executar:

```cpp
stable_sort(commandLists, by Id)
```

O `stable_sort` é importante: comandos com a mesma chave preservam a ordem original.

## Dois modos de ordenação

```text
Classifier
Sequential
```

### Classifier hash

Hash de 64 bits:

```text
bits 63..56 : view       (8 bits)
bits 55..53 : type       (3 bits)
bits 52..51 : classify   (2 bits)
bits 50..43 : index      (8 bits)
bits 42..27 : shader     (16 bits)
```

Os bits inferiores restantes ficam livres nessa revisão.

Entrada lógica:

```text
view
classify
index
shader
```

### Sequential hash

```text
bits 63..56 : view   (8 bits)
bits 55..53 : type   (3 bits)
bits 31..0  : index  (32 bits)
```

Use quando a ordem exata precisa ser preservada por sequência.

## Classes de render

```text
None
Opaque
PreAlpha
PostAlpha
```

Se blending está desativado:

```text
Opaque
```

Com blending, uma pequena tabela baseada no blend hash determina `PreAlpha` ou `PostAlpha`.

## PipelineInfo usada para classificar

O manager mantém, a partir do PSO atual:

```text
Shader
DepthWrite
BlendEnable
SrcBlend
DestBlend
BlendHash
```

Quando `RCommandListInfo.Classify == None`, o classificador calcula automaticamente a classe a partir desse estado.

## Resource transitions

Transitions globais acumuladas são aplicadas antes das command lists.

Transitions geradas por uploads dentro da lista são acumuladas e aplicadas imediatamente antes de `Draw/DrawIndexed`.

Ao `CommitShaderResources`:

```text
ShouldTransition=true -> TRANSITION
senão modo explícito fornecido
senão VERIFY
```

Após o primeiro commit:

```text
ShouldTransition=false
```

## Consequência para nosso renderer

Podemos reproduzir o conceito com segurança:

```text
RenderPacket generation
  -> order key
  -> stable sort
  -> state/resource batch
  -> backend command replay
```

OpenGL 4.6+ usa a mesma ordenação para reduzir program/state/texture switches.
Vulkan usa a mesma ordenação para reduzir PSO/descriptor switches e organizar transitions.

O ponto importante é manter um modo `Sequential` para passes onde transparência/efeitos dependem estritamente da ordem.
