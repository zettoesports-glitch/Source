# 31 — Executor de threads CPU do NextMU

Status: **CONFIRMADO** por assembly e RTTI do `NextMU.exe` x64.

## Inicialização

Função principal:

```text
0x14007F520
```

O startup chama essa rotina e, em falha, possui a mensagem:

```text
Failed to initialize threads.
```

## Quantidade de workers

O código chama `_Thrd_hardware_concurrency` e limita o resultado em 16:

```asm
0x14007F53A call _Thrd_hardware_concurrency
mov esi, eax
mov r14d, 0x10
cmp eax, r14d
cmova esi, r14d
```

Equivalente:

```cpp
workerCount = std::min(hardware_concurrency(), 16u);
```

## Criação dos workers

Para cada worker:

- aloca um bloco de 16 bytes;
- grava o índice do worker em `+0x00`;
- grava callback `0x14007F740` em `+0x08`;
- cria a thread por `_beginthreadex`;
- entry point real: `0x14007F1D0`.

O loop cria exatamente `workerCount` threads.

## Scheduler

Globals principais observados:

```text
0x1409CA548  shutdown/stop flag
0x1409CA550  worker/thread records begin
0x1409CA558  worker/thread records end
0x1409CA568  synchronization state A
0x1409CA570  synchronization state B
0x1409CA578  current executor/job object
```

O worker usa operações atômicas (`lock xadd`, `xchg`) e primitivas de espera/acordar para coordenar fases de trabalho.

## RTTI preservado

O executável possui especializações de executor para os sistemas do jogo, incluindo:

```text
NCharacters::Update
NCharacters::PreRender
NObjects::PreRender
NParticles::Update
NParticles::Render
NJoints::Update
NJoints::Render
```

com tipos como:

```text
NThreadExecutorIterator<...>
NThreadExecutorRangeIterator<...>
```

Portanto a preparação de render/update do MU é paralelizada no CPU.

## Distinção importante

A build não usa, no frame principal, os deferred contexts da Diligent para gravar comandos GPU em paralelo.

Arquitetura confirmada:

```text
                 CPU
          ┌──────┴──────┐
        worker 0 ... worker N
          │  Update / PreRender
          └──────┬──────┘
                 ↓
       NextMU RenderCommandBuffer
                 ↓
       immediate IDeviceContext
                 ↓
               GPU
```

Assim:

```text
CPU render preparation = multi-threaded
GPU command submission  = immediate/single-context
```

## Relevância para nosso MU

Esse é um modelo seguro para evolução:

1. paralelizar culling, pose preparation, command generation e effect building;
2. gerar `RenderPackets` thread-local ou por ranges;
3. mesclar/ordenar os packets;
4. manter submissão gráfica inicialmente em um único backend context;
5. só introduzir gravação GPU multi-threaded se métricas mostrarem benefício.

Isso reduz risco de sincronização no OpenGL 4.6 e mantém um caminho natural para Vulkan.
