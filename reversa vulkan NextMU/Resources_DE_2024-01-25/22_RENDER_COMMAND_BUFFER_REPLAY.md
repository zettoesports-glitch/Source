# 22 — Render command buffer/replay do NextMU

Status: **CONFIRMADO por jump-table + assembly x64 + interface Diligent v2.5.4**.

Esta é uma das peças arquiteturais mais importantes encontradas até agora.

O NextMU não precisa emitir toda operação de render imediatamente para `IDeviceContext`. Produtores de Model/terrain/etc. codificam comandos em uma fila própria, e uma rotina central posterior faz o replay para Diligent/Vulkan/D3D/OpenGL.

## Estrutura da fila

Cada record ocupa:

```text
0x70 = 112 bytes
```

O dispatcher percorre:

```asm
mov eax,[rsi]       ; command type
cmp eax,0xA         ; 0..10
jump table
...
add rsi,0x70
```

Logo existem **11 command types (0–10)**.

Dispatcher:

```text
0x14009A0F0 ... 0x14009A74A
```

Jump table:

```text
0x14009A74C
```

## Enum decodificado exatamente

A tabela contém os seguintes destinos:

| Type | Handler | Operação confirmada |
|---:|---:|---|
| 0 | `0x14009A2ED` | `IDeviceContext::UpdateBuffer` |
| 1 | `0x14009A3DC` | `MapBuffer` → memcpy → `UnmapBuffer` |
| 2 | `0x14009A487` | `IDeviceContext::UpdateTexture` |
| 3 | `0x14009A4EE` | resolver variável do SRB por nome → `IShaderResourceVariable::Set` |
| 4 | `0x14009A4EE` | mesmo handler de binding de recurso do type 3 |
| 5 | `0x14009A518` | `IDeviceContext::SetPipelineState` |
| 6 | `0x14009A533` | `IDeviceContext::SetVertexBuffers` |
| 7 | `0x14009A56C` | `IDeviceContext::SetIndexBuffer` |
| 8 | `0x14009A588` | `IDeviceContext::CommitShaderResources` |
| 9 | `0x14009A5C2` | flush transitions → `IDeviceContext::Draw` |
| 10 | `0x14009A608` | flush transitions → `IDeviceContext::DrawIndexed` |

### Vtable offsets usados

Diligent v2.5.4:

```text
+0x30 SetPipelineState
+0x40 CommitShaderResources
+0x58 SetVertexBuffers
+0x68 SetIndexBuffer
+0xA8 Draw
+0xB0 DrawIndexed
+0x148 UpdateBuffer
+0x158 MapBuffer
+0x160 UnmapBuffer
+0x168 UpdateTexture
+0x1A0 TransitionResourceStates
```

## Type 1 — Map/Unmap buffer

O handler:

```text
MapBuffer(buffer, mapType, mapFlags, ...)
copy bytes para mapped pointer
UnmapBuffer(buffer, mapType)
```

O Model usa esse command type para:

```text
ModelViewProj 128 bytes
ModelSettings  96 bytes
```

Isso corrige uma hipótese anterior: esses UBOs dinâmicos não usam `UpdateBuffer`; eles usam Map/Unmap.

## Types 3 e 4 — binding por nome

Ambos caem no mesmo handler:

```text
object->GetVariableByName(shaderType, name)
variable->Set(resource, flags=0)
```

Pelo offset `+0x38`, o objeto é compatível com `IShaderResourceBinding::GetVariableByName` da v2.5.4.

Os dois IDs têm o mesmo comportamento de replay; a diferença semântica na camada produtora ainda está em análise. Não atribuir nomes distintos sem provar os appenders/callers.

## Type 5 — Pipeline State

Appender confirmado:

```text
0x14009A860
```

grava:

```text
type = 5
```

No replay o record referencia o wrapper/handle de pipeline e chama:

```text
IDeviceContext::SetPipelineState
```

## Type 6 — Vertex buffers

Appender confirmado:

```text
0x14009A900
```

grava `type=6`.

Replay:

```text
SetVertexBuffers(...)
```

## Type 8 — Commit SRB

Appender confirmado:

```text
0x140099F70
```

grava `type=8`.

Replay chama `CommitShaderResources`. Existe uma flag no wrapper do binding que altera o transition mode/commit behavior e é zerada depois do commit.

## Types 9/10 — Draw

Appender de Draw confirmado:

```text
0x14009A020 -> type 9
```

Antes do draw, o dispatcher verifica o vetor de `StateTransitionDesc` acumulado. Se não estiver vazio:

```text
TransitionResourceStates(count, transitions)
```

Só então:

```text
Draw(...)
```

O type 10 faz o mesmo antes de `DrawIndexed(...)`.

## Transition batching

UpdateBuffer/UpdateTexture podem gerar descriptors de transition, que são acumulados numa área temporária. Em vez de transicionar imediatamente após cada upload:

```text
upload A
upload B
upload C
       ↓
acumula transitions
       ↓
antes do Draw/DrawIndexed
       ↓
TransitionResourceStates(N)
       ↓
draw
```

Isso reduz chamadas/barriers redundantes e mantém o frontend independente do backend.

## Arquitetura recuperada

```text
MU render producers
      │
      ├─ Model
      ├─ Terrain
      ├─ Objects/Characters
      ├─ Particle/Joint
      └─ UI/effects
      │
      ↓
112-byte RenderCommand records
      ↓
command queue
      ↓
central replay 0x14009A0F0
      │
      ├─ buffer/texture upload
      ├─ resource binding
      ├─ PSO
      ├─ VB/IB
      ├─ SRB commit
      ├─ barrier batching
      └─ Draw / DrawIndexed
      ↓
Diligent IDeviceContext
      ↓
Vulkan / D3D12 / D3D11 / OpenGL
```

## Valor para nosso MU

Essa é uma arquitetura altamente recomendável para um frontend único GL4.6+/Vulkan:

1. gameplay/renderers não conhecem APIs gráficas;
2. produzem comandos independentes de backend;
3. command compiler pode remover estado redundante;
4. barriers/transitions ficam centralizados no Vulkan;
5. OpenGL pode traduzir os mesmos commands para state-cache + draws;
6. permite telemetria de draw calls, uploads e state changes;
7. facilita futuras otimizações (sorting seguro, batching, indirect) sem alterar gameplay.

## O que ainda está em análise

- diferença semântica entre command types 3 e 4;
- formato campo-a-campo de todos os 112 bytes por type;
- producer/helper de cada command type;
- regras completas de deduplicação/ordering antes do replay.
