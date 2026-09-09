# 29 — Frame scheduling e modelo de contexto

Status: **CONFIRMADO para a camada de jogo analisada**.

A tabela de `IDeviceContext` v2.5.4 foi usada para procurar calls indiretos no intervalo do código do NextMU.

## Não observado no frame/gameplay

Não foram encontrados callsites do jogo para:

```text
+0x108 FinishCommandList
+0x110 ExecuteCommandLists
+0x118 EnqueueSignal
+0x120 DeviceWaitForFence
+0x128 WaitForIdle
+0x130 BeginQuery
+0x138 EndQuery
+0x190 FinishFrame
+0x198 GetFrameNumber
```

Há ocorrências desses offsets em regiões grandes de bibliotecas/DiligentFX, mas não na camada principal do jogo.

## Observado repetidamente

```text
+0x1A0 TransitionResourceStates
```

é usado por Terrain/effects/Particles/Joints e outros caminhos do renderer.

## Arquitetura resultante

A build de 25/01/2024 trabalha essencialmente como:

```text
CPU/game systems
    -> command buffer próprio do NextMU
    -> immediate IDeviceContext
    -> batched TransitionResourceStates
    -> draw calls
    -> SwapChain::Present
```

Não há evidência de que o gameplay dependa de deferred contexts/command lists da Diligent.

## Implicação para nosso MU

Para primeira implementação Vulkan estável, não precisamos introduzir multithreaded command recording. Podemos reproduzir a arquitetura mais simples e validada:

1. frontend gera comandos;
2. agrupa transições;
3. immediate context/backend executa;
4. otimizações multi-thread entram depois de métricas.
