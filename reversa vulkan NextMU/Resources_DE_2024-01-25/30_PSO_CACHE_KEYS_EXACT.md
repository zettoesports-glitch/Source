# 30 — PSO cache e chaves exatas

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64.

## Funções principais

```text
0x140097380  builder / criação e inserção de PSO
0x140097BC0  lookup do cache
0x1409CA8B0  árvore global externa
```

O NextMU não cria `GraphicsPipelineState` por draw. Ele usa um cache hierárquico em dois níveis e só chama `IRenderDevice::CreateGraphicsPipelineState` (`vtable +0x48`) em cache miss.

## Chave externa — 30 bits

Derivada dos quatro primeiros bytes do descritor de pipeline:

```cpp
uint32_t outerKey =
    uint16_t(desc[0] | (desc[1] << 8)) |
    ((uint16_t(desc[2] | (desc[3] << 8)) & 0x3FFFu) << 16);
```

Assembly em `0x140097BCC..0x140097BE5`:

```text
movzx eax, word [rcx]
movzx r8d, word [rcx+2]
and   r8d, 0x3fff
shl   r8d, 16
or    r8d, eax
```

Essa chave identifica a família base do pipeline (programa/layout e metadados associados). A decomposição semântica fina dos 30 bits ainda deve ser tratada como reconstrução até todos os producers serem mapeados.

## Chave interna — 61 bits

O segundo nível da árvore fica em `outerNode +0x28`.

A chave é montada dos 10 bytes do render-state exatamente assim:

```cpp
uint64_t PackRenderState(const uint8_t s[10])
{
    uint64_t key = s[9] & 0x07;
    key = (key << 8) | s[8];
    key = (key << 5) | (s[7] & 0x1F);
    key = (key << 5) | (s[6] & 0x1F);
    key = (key << 5) | (s[5] & 0x1F);
    key = (key << 8) | s[4];
    key = (key << 8) | s[3];
    key = (key << 8) | s[2];
    key = (key << 5) | (s[1] & 0x1F);
    key = (key << 6) | (s[0] & 0x3F);
    return key;
}
```

Total:

```text
3 + 8 + 5 + 5 + 5 + 8 + 8 + 8 + 5 + 6 = 61 bits
```

Assembly: `0x140097C23..0x140097C9C`.

## O estado não é apenas um hash

O builder em `0x140097380` volta a ler esses 10 bytes e os converte em campos do `GraphicsPipelineStateCreateInfo`.

Exemplos diretamente visíveis:

```text
state[0] low bits -> configuração raster/depth auxiliar
state[1] low nibble + bit 4 -> depth/comparison flags
state[2] -> campo gráfico de 8 bits
state[3] nibbles -> dois valores de blend
state[4] nibbles -> dois valores de blend
state[5..7] low 5 bits -> outros estados de blend/raster/depth
state[8] -> campo de 8 bits
state[9] low 3 bits -> estado final compacto
```

Os nomes exatos de cada byte só devem ser promovidos para CONFIRMADO ao cruzar todos os producers (`model.json`, Terrain, Particle, Joint etc.).

## Arquitetura

```text
pipeline identity (30 bits)
        ↓
outer RB-tree
        ↓
render state (61 bits)
        ↓
inner RB-tree
        ↓
cached PSO + metadata/SRB state
```

## Aplicação no nosso MU

Devemos reproduzir o conceito, não necessariamente a mesma codificação:

```cpp
struct PipelineKey
{
    ProgramId program;
    VertexLayoutId layout;
    RenderStateKey state;
    RenderTargetFormatKey formats;
};
```

Assim OpenGL 4.6+ pode usar a mesma chave para cachear `GLProgram + state objects`, enquanto Vulkan usa a chave para `VkPipeline`/PSO. Isso elimina criação/reconfiguração repetitiva por mesh.
