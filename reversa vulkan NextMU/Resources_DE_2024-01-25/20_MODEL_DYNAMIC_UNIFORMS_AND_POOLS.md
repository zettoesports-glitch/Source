# 20 — Model dynamic uniforms e CPU backing queues

Status: **CONFIRMADO-CRUZADO** pelo assembly e pela source pública de 25/01/2024.

## Correção importante

A interpretação inicial de “páginas GPU” estava incorreta. `NResizableQueue<T, 1024>` é uma **arena/fila de memória CPU thread-safe** usada para manter os structs vivos até o command buffer executar o `Map/Discard + memcpy`.

Os buffers GPU são apenas dois UBOs dinâmicos pequenos:

```text
ModelViewUniform     128 B
ModelSettingsUniform  96 B
```

## NResizableQueue exata

Template:

```cpp
NResizableQueue<Type, Incr = 1024>
```

Comportamento:

- começa com 1 bloco CPU de `1024 * sizeof(Type)`;
- `Buffers.reserve(50)` reserva capacidade do vetor de ponteiros, não 50 blocos;
- `Allocate()` é protegido por mutex;
- quando o bloco atual atinge 1024 itens, reutiliza o próximo bloco já existente ou aloca outro;
- `Reset()` apenas volta `Group=0`, `Index=0` para reutilização no frame seguinte;
- memória é liberada somente no destrutor.

## ModelView CPU backing

```text
NModelViewSettings = 128 B
1024 structs/bloco  = 128 KiB por bloco CPU
```

Estrutura:

```cpp
struct NModelViewSettings
{
    glm::mat4 Model;    // 64 B
    glm::mat4 ViewProj; // 64 B
};
```

Por draw:

1. `ModelViewBuffer.Allocate()` retorna um slot CPU;
2. grava Model + ViewProj;
3. o command manager recebe `UpdateBufferWithMap`;
4. no replay, faz Map/Discard no **mesmo UBO GPU de 128 B** e copia o slot CPU.

## ModelSettings CPU backing

```text
NModelSettings = 96 B
1024 structs/bloco = 96 KiB por bloco CPU
```

A mesma estratégia é usada:

1. aloca struct CPU;
2. preenche os 96 B;
3. enfileira `UpdateBufferWithMap`;
4. no replay, Map/Discard no UBO GPU de 96 B;
5. copia os dados daquele draw imediatamente antes do PSO/resources/draw correspondentes.

## Por que a arena CPU existe

O renderer não pode apontar o command buffer para um struct temporário de stack que desapareça antes de `Execute()`. A `NResizableQueue` fornece endereços estáveis durante todo o frame e ainda permite produtores CPU multithread em outros subsistemas.

## Fluxo real

```text
CPU backing slot 128 B
 -> enqueue Map/Discard(ModelViewUniform)
CPU backing slot 96 B
 -> enqueue Map/Discard(ModelSettingsUniform)
 -> SetPipelineState
 -> SetVertexBuffer
 -> CommitShaderResources
 -> Draw
```

## Aplicação no nosso renderer

Podemos manter o mesmo conceito de **FrameUploadArena CPU**, mas melhorar a camada GPU:

- OpenGL 4.6+: UBO ring/buffer storage + range binding;
- Vulkan: persistently mapped upload/ring buffer + dynamic offsets;
- manter uma área CPU estável para command packets produzidos por workers;
- evitar um Map/Discard por draw quando a implementação moderna puder escrever vários uniforms em uma única arena GPU.

Portanto o conceito a copiar é **arena CPU estável + command recording**, não “1024 UBOs/página”.
