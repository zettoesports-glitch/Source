# 19 — Render command buffer/replay exato

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 e pela ordem de vtable da Diligent Engine v2.5.4.

## Replay central

Função principal observada:

```text
0x14009A0F0
```

O frontend acumula comandos gráficos e executa posteriormente no `IDeviceContext`. Cada registro ocupa:

```text
0x70 = 112 bytes
```

O dispatcher aceita **11 tipos (0..10)**.

## Tipos confirmados

```text
0  UpdateBuffer
1  MapBuffer + memcpy + UnmapBuffer
2  UpdateTexture
3  GetVariableByName + Set resource
4  GetVariableByName + Set resource (mesmo executor; semântica do produtor ainda separada)
5  SetPipelineState
6  SetVertexBuffers
7  SetIndexBuffer
8  CommitShaderResources
9  TransitionResourceStates + Draw
10 TransitionResourceStates + DrawIndexed
```

### Endereços do jump table

```text
0  -> 0x14009A2ED
1  -> 0x14009A3DC
2  -> 0x14009A487
3  -> 0x14009A4EE
4  -> 0x14009A4EE
5  -> 0x14009A518
6  -> 0x14009A533
7  -> 0x14009A56C
8  -> 0x14009A588
9  -> 0x14009A5C2
10 -> 0x14009A608
```

## Correspondência Diligent v2.5.4

```text
IDeviceContext +0x30  SetPipelineState
IDeviceContext +0x40  CommitShaderResources
IDeviceContext +0x58  SetVertexBuffers
IDeviceContext +0x68  SetIndexBuffer
IDeviceContext +0xA8  Draw
IDeviceContext +0xB0  DrawIndexed
IDeviceContext +0x148 UpdateBuffer
IDeviceContext +0x158 MapBuffer
IDeviceContext +0x160 UnmapBuffer
IDeviceContext +0x168 UpdateTexture
IDeviceContext +0x1A0 TransitionResourceStates
```

## Helpers de produção já identificados

```text
0x140099F70 -> append type 8  / CommitShaderResources
0x14009A020 -> append type 9  / Draw
0x14009A860 -> append type 5  / SetPipelineState
0x14009A900 -> append type 6  / SetVertexBuffers
0x14009A9B0 -> append type 1  / MapBuffer + memcpy + UnmapBuffer
```

O produtor específico de type 10 ainda será marcado quando o xref for isolado; o executor de `DrawIndexed` já está diretamente provado.

## Resource transitions

Antes de `Draw` ou `DrawIndexed`, o replay verifica uma lista de transições pendentes e chama `TransitionResourceStates()` uma única vez para o lote.

Os registros de transição têm:

```text
0x30 = 48 bytes
```

O contador é derivado pela diferença de ponteiros dividida por 48.

Ao terminar o replay, as filas são reaproveitadas/zeradas em vez de reconstruídas a cada comando.

## Arquitetura reconstruída

```text
Gameplay / render systems
        ↓
Render command encoder
        ↓
112-byte command records
        ↓
PSO / SRB / buffer / upload commands
        ↓
transition list (48-byte records)
        ↓
central replay
        ↓
IDeviceContext
        ↓
Vulkan / D3D11 / D3D12 / OpenGL
```

## Valor para nosso MU

Este é um dos melhores sistemas recuperados para nosso renderer. Em vez de espalhar chamadas OpenGL/Vulkan por BMD, Terrain, Particles e UI, podemos fazer todos emitirem um frontend comum e executar no backend no final.

Benefícios esperados:
- batching de resource transitions;
- menos chamadas de driver pequenas;
- state/PSO cache central;
- possibilidade de ordenar apenas comandos seguros;
- mesmo frontend para OpenGL 4.6+ e Vulkan;
- telemetria por tipo de comando;
- caminho claro para multithreading futuramente.

Não é a source original: é o contrato de comportamento reconstruído a partir das chamadas reais do binário.