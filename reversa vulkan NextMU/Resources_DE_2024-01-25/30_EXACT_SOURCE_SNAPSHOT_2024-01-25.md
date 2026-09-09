# 30 — Snapshot público de source da mesma data

Status: **CONFIRMADO / BASELINE DE REFERÊNCIA**.

## Commit encontrado

O repositório público `kuncarous/nextmu` preserva um commit exatamente em **2024-01-25**:

```text
commit  12f30a1c22b99bd59020cc7fd00d5910bed2cc17
date    2024-01-25T01:35:12Z
message added rmlui full support
tree    7bdcfc69f63a8b33dfb5954f1c78d6c59d8cd5aa
```

O pacote analisado é `Resources_DE_2024-01-25.zip`.

## Relação com o binário

A source da mesma data reproduz os mesmos contratos que foram recuperados antes por análise estática do `NextMU.exe`:

- `MUGraphics::InitializeEngine`;
- D3D11 / Vulkan / D3D12 / OpenGL;
- Diligent API v2.5.4;
- `GetEngineFactoryVk`;
- `CreateDeviceAndContextsVk`;
- `CreateSwapChainVk`;
- resource names de Model/Terrain/Particle/Joint/RmlUI;
- input layouts;
- command manager;
- PSO cache;
- SRB cache;
- Model uniform sizes 128/96;
- Particle 50.000;
- Joint 250.000.

Portanto esta source é usada como **baseline pública da mesma data**, cruzada com o executável fornecido pelo usuário.

## Regra de confiança

```text
CONFIRMADO-BINÁRIO  = observado diretamente no EXE/DLL
CONFIRMADO-SOURCE   = presente no commit de 25/01/2024
CONFIRMADO-CRUZADO  = source e binário concordam
RECONSTRUÍDO        = implementação clean-room baseada nos contratos acima
INFERIDO            = hipótese ainda não comprovada
```

Não se deve substituir a verificação do executável apenas pela source pública: diferenças de build/configuração ainda são possíveis.

## Build flags relevantes da snapshot

```text
NEXTMU_UI_LIBRARY             = NEXTMU_UI_RMLUI
NEXTMU_COMPRESSED_MESHS       = 0
NEXTMU_COMPRESSED_PARTICLES   = 0
NEXTMU_COMPRESSED_JOINTS      = 0
NEXTMU_RENDER_BBOX            = 0
```

Particle e Joint possuem update/render CPU multithread habilitados.

## Consequência

A engenharia passa a concentrar esforço em:

1. confirmar quais trechos da snapshot entraram no binário;
2. documentar configurações exatas da build;
3. extrair arquitetura reutilizável para nosso OpenGL 4.6+/Vulkan;
4. evitar decompilar manualmente código que já existe publicamente na mesma versão temporal.
