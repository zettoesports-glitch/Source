# 32 — Cache compartilhado de RenderState / PSO / SRB

Status: **CONFIRMADO estruturalmente** por assembly do `NextMU.exe` x64. Os nomes semânticos de cada campo da chave continuam separados de fatos já provados.

Função central:

```text
0x14004A0E0
```

Registry global observado:

```text
0x1409CA810
```

Model e Terrain chamam esta mesma função para obter/reutilizar objetos de render state/pipeline.

## Estrutura de busca

A função não usa uma chave inteira plana.

Ela realiza, em sequência:

1. busca em uma árvore ordenada pelo primeiro inteiro (`edx`);
2. dentro do nó encontrado, busca em outra árvore ordenada pelo segundo inteiro (`r9d`);
3. percorre uma coleção/vector de objetos candidatos;
4. compara identidade textual/string armazenada no objeto com a string/span recebida;
5. retorna o objeto existente quando encontra correspondência;
6. caso contrário entra no caminho de criação `0x14004A260`, que desce para `0x140048E00`.

## Interface observada

Forma aproximada baseada na calling convention x64:

```cpp
CacheObject* FindOrCreate(
    Registry* registry,      // rcx
    uint32_t primaryKey,     // edx
    StringContext context,   // r8
    uint32_t secondaryKey,   // r9d
    const uint32_t* tuple    // stack arg
);
```

O tuple possui comprimento derivado do `secondaryKey`/contador recebido e é usado durante criação/identificação. Não devemos renomear cada elemento como blend/depth/shadow até o caller provar a semântica.

## Busca exata observada

No primeiro nível:

```text
node key at +0x20
```

No segundo nível:

```text
nested node key at +0x20
```

Depois, a coleção de objetos é percorrida e a string do candidato é comparada com a entrada.

Isto confirma uma chave composta do tipo:

```text
numeric family/program
    + numeric variant/domain
    + tuple de estados
    + string identity/program/layout
```

## Objeto retornado

Campos já observados no uso:

```text
+0x08  resources_initialized flag
+0x09  SRB dirty / commit-transition flag
+0x28  Shader Resource Binding (SRB)
```

O command type 8 (`CommitShaderResources`) consome `object +0x28` e zera `object +0x09` depois do commit.

## Model

O Model chama `0x14004A0E0` em torno de `0x14008DB2A`.
A chave varia de acordo com IDs/estado do programa/material e com a variante de shadow/render.

O objeto retornado é então usado para:

```text
SetPipelineState
bind g_VertexTexture / g_Texture / shadows
CommitShaderResources
Draw
```

## Terrain

O Terrain chama o mesmo cache em múltiplos pontos, incluindo aproximadamente:

```text
0x140051A5E
0x140051BB5
0x140051F69
0x140051FE9
```

Isso demonstra que os passes/variantes de Terrain também reutilizam PSO/SRB, em vez de recriar pipeline por chunk.

## Por que isso importa para nosso renderer

Um equivalente limpo para GL4.6+/Vulkan deve gerar uma chave imutável de pipeline, por exemplo:

```text
shader/program id
vertex layout
primitive topology
raster state
blend state
depth/stencil state
render-target formats
shadow/material variant
```

E manter bindings de material/texturas em um objeto separado/reutilizável.

A referência NextMU mostra claramente a intenção:

```text
config/material state
       ↓
cache lookup
       ↓
reuse PSO + SRB
       ↓
command buffer
       ↓
backend
```

Isso reduz criação de pipelines e state churn, especialmente em Vulkan.