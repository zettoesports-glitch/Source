# 13 — Characters, Objects e arquitetura de render do MU

Status: **RTTI/component access CONFIRMADO; detalhes de shader/material ainda em expansão.**

## ECS

O NextMU usa EnTT para organizar entidades de render.

Componentes base confirmados:

```text
NEntity::NRenderable
NEntity::NRenderState
```

Hashes exatos usados no registry:

```text
struct NEntity::NRenderable  = 0x3297031D
struct NEntity::NRenderState = 0x95F05ABE
```

## Characters

```text
NCharacters::PreRender(const NRenderSettings&)
0x1400678E0 - 0x140067E5F
```

Lambda RTTI vftable:

```text
0x1404834F0
xref: 0x140067D51
```

O `PreRender` consulta storages de `NRenderable`/`NRenderState`, mostrando que estado visual é preparado antes do draw efetivo.

## Objects

```text
NObjects::PreRender(const NRenderSettings&)
0x1400A5A20 - 0x1400A6216
```

Duas lambdas localizadas:

```text
vftable 0x140486070 -> xref 0x1400A5E31
vftable 0x1404860E8 -> xref 0x1400A6165
```

Também consulta `NRenderable` e `NRenderState`.

## Interpretação arquitetural segura

O padrão suporta pipeline em duas fases:

```text
simulation/update
      ↓
PreRender
  - resolve visibilidade/estado
  - prepara NRenderable/NRenderState
  - cria/agrupa informações de draw
      ↓
render pass/backend
```

A existência comprovada de `PreRender` separado é importante para nosso MU: lógica de personagem/objeto não precisa atualizar state GPU no meio de cada draw.

## Relação com nosso OpenGL 4.6+/Vulkan

```text
MU entity/model state
        ↓
RenderPacket / MaterialPacket
        ↓
Backend abstraction
    ├── OpenGL 4.6+
    └── Vulkan
```

Chrome/BlendMesh/Bones/alpha/fog podem ser calculados no frontend e traduzidos para bindings do backend.

## Próximos detalhes a fechar

- consumidor final do `NRenderable` de characters/objects;
- layouts de vertex/index de model;
- constant/uniform buffers;
- material/texture binding;
- PSOs opaque/alpha/transparent/shadow;
- bone upload/pose reuse no NextMU;
- chaves exatas usadas no agrupamento pré-draw.

Esses itens permanecem explicitamente pendentes e não serão inferidos como fatos.
