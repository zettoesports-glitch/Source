# 20 — Model dynamic uniforms e pools

Status: **CONFIRMADO** pelo assembly do model renderer (`0x14008DC21+`).

## Estratégia

O NextMU não cria um buffer/uniform por draw. Ele usa páginas reutilizáveis e subaloca blocos fixos por mesh/draw, depois enfileira o upload no command buffer central.

## ModelViewProj

Pool observado:

```text
page size       0x20000 = 131072 bytes = 128 KiB
slots/page      0x400   = 1024
slot stride     0x80    = 128 bytes
```

Identidade exata:

```text
1024 * 128 = 131072 bytes
```

Cada slot recebe dois blocos de 64 bytes:

```text
offset 0x00..0x3F : matriz 4x4 #1
offset 0x40..0x7F : matriz 4x4 #2
```

O primeiro bloco vem de uma matriz já disponível no contexto do objeto; o segundo é obtido por uma função global de matriz. A nomenclatura semântica exata das duas matrizes permanece em investigação, portanto não é rotulada como `model/view/projection` individual sem prova adicional.

Upload:

```text
0x14009A9B0
command type 1
MapBuffer -> memcpy(128) -> UnmapBuffer
```

## ModelSettings

Segundo pool:

```text
page size       0x18000 = 98304 bytes = 96 KiB
slots/page      0x400   = 1024
slot stride     0x60    = 96 bytes
```

Identidade:

```text
1024 * 96 = 98304 bytes
```

Upload igualmente enfileirado via command type 1:

```text
MapBuffer -> memcpy(96) -> UnmapBuffer
```

## Gestão das páginas

As páginas são mantidas em arrays/vetores globais. Quando o índice de slot chega a 1024:
1. avança para a próxima página existente;
2. se não houver página, aloca uma nova;
3. reinicia o índice de slot;
4. mantém a página para reutilização futura.

Isso evita `new/delete` e criação de buffer por draw.

## Fluxo por mesh

```text
obter slot ModelViewProj (128 B)
  ↓
copiar 2 matrizes
  ↓
enfileirar upload
  ↓
obter slot ModelSettings (96 B)
  ↓
preencher material/estado
  ↓
enfileirar upload
  ↓
SetPipelineState
  ↓
SetVertexBuffers
  ↓
CommitShaderResources
  ↓
Draw
```

## Aplicação no nosso renderer

Vale reproduzir o conceito como um `DynamicUniformArena` compartilhado:

```cpp
struct UniformSlice {
    BufferHandle buffer;
    uint32_t offset;
    uint32_t size;
};
```

Com páginas persistentes e alinhamento por backend. No Vulkan podemos usar dynamic uniform/storage offsets; no GL4.6 podemos usar UBO range/buffer storage mantendo o mesmo frontend.

Este desenho é especialmente útil para nosso BMD, pois elimina centenas/milhares de pequenas atualizações isoladas.