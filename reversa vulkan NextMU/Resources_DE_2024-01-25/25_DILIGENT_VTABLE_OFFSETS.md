# 25 — Diligent v2.5.4 vtable offsets usados na reversa

Status: **CONFIRMADO** contra os headers públicos da Diligent Engine v2.5.4 e xrefs do `NextMU.exe` x64.

A tabela é usada para transformar chamadas indiretas sem símbolos em nomes concretos.

## IDeviceContext x64

```text
+0x20 GetDesc
+0x28 Begin
+0x30 SetPipelineState
+0x38 TransitionShaderResources
+0x40 CommitShaderResources
+0x48 SetStencilRef
+0x50 SetBlendFactors
+0x58 SetVertexBuffers
+0x60 InvalidateState
+0x68 SetIndexBuffer
+0x70 SetViewports
+0x78 SetScissorRects
+0x80 SetRenderTargets
+0x88 SetRenderTargetsExt
+0x90 BeginRenderPass
+0x98 NextSubpass
+0xA0 EndRenderPass
+0xA8 Draw
+0xB0 DrawIndexed
+0xB8 DrawIndirect
+0xC0 DrawIndexedIndirect
+0xC8 DrawMesh
+0xD0 DrawMeshIndirect
+0xD8 DispatchCompute
+0xE0 DispatchComputeIndirect
+0xE8 DispatchTile
+0xF0 GetTileSize
+0xF8 ClearDepthStencil
+0x100 ClearRenderTarget
+0x108 FinishCommandList
+0x110 ExecuteCommandLists
+0x118 EnqueueSignal
+0x120 DeviceWaitForFence
+0x128 WaitForIdle
+0x130 BeginQuery
+0x138 EndQuery
+0x140 Flush
+0x148 UpdateBuffer
+0x150 CopyBuffer
+0x158 MapBuffer
+0x160 UnmapBuffer
+0x168 UpdateTexture
+0x170 CopyTexture
+0x178 MapTextureSubresource
+0x180 UnmapTextureSubresource
+0x188 GenerateMips
+0x190 FinishFrame
+0x198 GetFrameNumber
+0x1A0 TransitionResourceStates
+0x1A8 ResolveTextureSubresource
```

## IEngineFactoryVk x64

```text
+0x50 CreateDeviceAndContextsVk
+0x58 CreateSwapChainVk
+0x60 EnableDeviceSimulation
```

## IPipelineState / SRB — métodos relevantes

O `NextMU` usa a interface v2.5.4 para:

```text
IPipelineState::GetStaticVariableByName
IPipelineState::CreateShaderResourceBinding
IShaderResourceBinding::GetVariableByName
IShaderResourceVariable::Set
```

A posição absoluta é sempre validada contra o layout completo da interface antes de rotular um callsite.

## Regra de uso

Não inferir método apenas pelo valor do offset isolado. Confirmar:
1. interface base do objeto no registrador;
2. versão Diligent (v2.5.4);
3. ordem dos métodos no header;
4. argumentos montados ao redor do `call [vtable+offset]`.

Esta tabela reduz drasticamente falsos positivos na análise dos renderers MU.
