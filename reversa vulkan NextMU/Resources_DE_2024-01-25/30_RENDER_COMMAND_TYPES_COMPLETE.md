# 30 — RenderCommandBuffer: mapa completo dos 11 tipos

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64 + ABI Diligent Engine v2.5.4.

Executor principal: `0x14009A0F0`.
Cada comando ocupa **0x70 / 112 bytes**.
Jump table: `0x14009A74C`.

## Tipos

```text
0  -> UpdateBuffer
1  -> MapBuffer + memcpy + UnmapBuffer
2  -> UpdateTexture
3  -> SRB::GetVariableByName(stage,name) + ShaderResourceVariable::Set(object)
4  -> mesmo caminho de execução do tipo 3
5  -> SetPipelineState
6  -> SetVertexBuffers
7  -> SetIndexBuffer
8  -> CommitShaderResources
9  -> Draw
10 -> DrawIndexed
```

## Endereços dos handlers no executor

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

## Diligent ABI usada

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

`IObject` possui quatro métodos base. Com isso também ficam confirmados:

```text
IShaderResourceBinding +0x38 = GetVariableByName
IShaderResourceVariable +0x20 = Set
```

Portanto tipos 3/4 são bindings reais por nome dentro da fila, não uma interpretação aproximada.

## Producers já identificados

```text
0x140099F70 = enqueue tipo 8 / CommitShaderResources
0x14009A020 = enqueue tipo 9 / Draw
0x14009A860 = enqueue tipo 5 / SetPipelineState
0x14009A900 = enqueue tipo 6 / SetVertexBuffers
0x14009A9B0 = enqueue tipo 1 / MapBuffer + upload
```

Os producers de 0,2,3,4,7,10 ainda estão sendo nomeados por xref. Os handlers de execução, porém, já estão confirmados.

## Transições antes dos draws

Antes de executar comando 9 ou 10, o executor verifica a lista acumulada de transições e chama:

```cpp
IDeviceContext::TransitionResourceStates(...);
```

As transições usam registros de **48 bytes**. O lote é esvaziado antes do draw.

## CommitShaderResources

No tipo 8:

- o comando aponta para um objeto de cache/render-state;
- `object + 0x28` contém o SRB;
- `object + 0x09` funciona como dirty/force-transition flag;
- quando `+0x09 != 0`, o transition mode usado no commit é forçado para `1`;
- caso contrário o comando usa seu mode armazenado ou o default `2`;
- depois do commit, `object + 0x09` é zerado.

## Draw vs DrawIndexed

O Model/BMD moderno usa **Draw**, pois a geometria é expandida em triangle-list no vertex buffer.
Particle/Joint e outros caminhos com index buffer podem usar **DrawIndexed**.

## Arquitetura resultante

```text
Gameplay / render systems
        ↓
RenderCommandBuffer do NextMU
        ↓
resource-transition batching
        ↓
Immediate IDeviceContext
        ↓
Vulkan / D3D12 / D3D11 / OpenGL
```

Essa é uma das peças mais importantes para reproduzir o frontend no nosso MU sem acoplar gameplay diretamente ao backend.