# 20 — Model UBOs e pool de ModelSettings

Status: **CONFIRMADO por assembly x64**.

## UBOs globais do Model

O renderer cria dois buffers dinâmicos principais.

### ModelViewProj

Global:

```text
0x1409CA620
```

Descriptor:

```text
Size      = 0x80 = 128 bytes
Bind      = BIND_UNIFORM_BUFFER
Usage     = USAGE_DYNAMIC
CPUAccess = CPU_ACCESS_WRITE
ContextMask = 1
```

No draw path, o renderer enfileira exatamente 128 bytes para atualização.

### ModelSettings

Global:

```text
0x1409CA628
```

Descriptor:

```text
Size      = 0x60 = 96 bytes
Bind      = BIND_UNIFORM_BUFFER
Usage     = USAGE_DYNAMIC
CPUAccess = CPU_ACCESS_WRITE
ContextMask = 1
```

No draw path, o renderer enfileira exatamente 96 bytes.

## Importante: atualização é por Map/Unmap

O helper usado pelos dois (`0x14009A9B0`) cria **render command type 1**.

O dispatcher do command buffer prova que type 1 executa:

```text
IDeviceContext::MapBuffer
memcpy
IDeviceContext::UnmapBuffer
```

Portanto uma hipótese anterior de `UpdateBuffer` para esses dois UBOs estava incorreta. O caminho confirmado é **Map→copy→Unmap** de buffer dinâmico.

## Pool CPU-side de ModelSettings

O renderer mantém um pool reutilizável para os records de 96 bytes usados por draw.

Valores recuperados:

```text
page allocation = 0x18000 = 98.304 bytes
record stride    = 0x60    = 96 bytes
records/page     = 0x400   = 1024
```

Verificação:

```text
96 × 1024 = 98.304 bytes
```

Quando a página atual atinge 1024 records, uma nova página de 98.304 bytes é alocada e adicionada ao vetor de páginas.

Isso evita `new/delete` ou heap allocation por mesh/draw.

## Arquitetura reconstruída

```text
Draw de modelo
   ↓
obter slot ModelSettings de 96 bytes no pool
   ↓
preencher state/material/light/etc.
   ↓
encode MapBuffer command para ModelViewProj
encode MapBuffer command para ModelSettings
   ↓
command replay
   ↓
Map/Unmap UBOs
   ↓
PSO/SRB/Draw
```

## O que ainda está sendo recuperado

A estrutura física completa de `ModelSettings` ainda não está nomeada campo a campo. O assembly mostra writes até pelo menos `+0x50`, mas só campos com semântica comprovada devem receber nomes definitivos.

## Recomendação para nosso renderer

Reproduzir a estratégia com um allocator por frame:

```text
FrameArena<ModelSettings>
```

em vez de heap por draw. Para Vulkan moderno, podemos evoluir isso para ring-buffer/UBO dinâmico com offsets; para GL4.6, UBO ring/persistent mapping. Primeiro preservar a semântica estável, depois otimizar.
