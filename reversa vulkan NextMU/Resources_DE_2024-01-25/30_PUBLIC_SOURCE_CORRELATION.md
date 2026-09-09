# 30 — Correlação com source pública histórica do NextMU

Status: **SOURCE-CORRELATED / ALTA CONFIANÇA**

## Baseline pública encontrada

Repositório:

```text
kuncarous/nextmu
```

Commit candidato:

```text
12f30a1c22b99bd59020cc7fd00d5910bed2cc17
2024-01-25 01:35:12 UTC
added rmlui full support
```

Esse commit é do mesmo dia identificado pelo pacote `Resources_DE_2024-01-25`.

## Correlações já verificadas contra o binário

A source histórica contém os mesmos contratos já recuperados por assembly/strings:

```text
MUGraphics::InitializeEngine
TransactionsCount
AdapterID / AdapterType
EngineVkCreateInfo
GetEngineFactoryVk
CreateDeviceAndContextsVk
CreateSwapChainVk
UNASSIGNED-CoreValidation-Shader-OutputNotConsumed
ModelViewProj
ModelSettings
g_SkeletonTexture
g_VertexTexture
g_Texture
TerrainSettings
ParticleSettings
JointSettings
```

A regra de escolha de adapter também explica o assembly observado:

```text
1. AdapterID explícito, se válido
2. AdapterType solicitado, se disponível
3. Caso automático: preferir Discrete > Integrated > Software
4. Em empate de tipo: escolher maior memória total
```

Memória comparada:

```text
LocalMemory + HostVisibleMemory + UnifiedMemory
```

## Importante: não assumir identidade total

Foi encontrada uma diferença: o binário analisado contém evidência de um registry/layout `grass`, enquanto `t_graphics_layouts.cpp` neste commit histórico não registra `grass`.

Portanto:

```text
commit 12f30... = baseline source extremamente próxima
não = prova de identidade byte-a-byte da árvore inteira
```

Cada módulo deve continuar sendo promovido para `SOURCE-CORRELATED` somente após bater com o binário.

## Licença

O repositório público usa licença MIT (Copyright 2022 kuncarous). Qualquer incorporação substancial da source pública deve preservar o aviso de copyright e a licença MIT.

## Estratégia daqui em diante

1. usar a source histórica para recuperar nomes/tipos/algoritmos;
2. conferir contra assembly/strings/estruturas do EXE;
3. marcar diferenças;
4. documentar contratos no nosso repositório;
5. criar implementação própria limpa para nosso frontend GL4.6+/Vulkan.
