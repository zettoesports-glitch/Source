# 30 — Configuração gráfica exata do NextMU

Status: **CONFIRMADO** por análise estática do `x64/NextMU.exe` do pacote `Resources_DE_2024-01-25`.

## Região principal

O carregamento/validação das opções gráficas fica na família de funções em torno de `0x140052A20+`.

Getters localizados:

```text
0x140052950  Shadows
0x140052960  ShadowBestCascadeSearch
0x140052970  ShadowCascadesCount
0x140052980  ShadowFarZ
0x140052990  ShadowFilterAcrossCascades
0x1400529A0  ShadowFilterSize
0x1400529B0  ShadowMode
0x1400529C0  ShadowPartitioning
0x1400529D0  ShadowResolution
0x1400529E0  ShadowRightHanded
0x1400529F0  VerticalSync
```

## Opções e limites observados

```text
Shadows                    bool
ShadowMode                 int    1..4      default 1
ShadowFarZ                 float  1000..50000 default 5000
ShadowRightHanded          bool
ShadowCascadesCount        int    1..8      default 4
ShadowResolution           int    1024..8192 default 2048
ShadowPartitioning         float  0..1      default 0.95
ShadowFilterSize           int    1..4      default 2
ShadowFilterAcrossCascades bool
ShadowBestCascadeSearch    bool
Antialiasing               bool
VerticalSync               bool
```

Constantes confirmadas no binário:

```text
0x1404822F4 = 1000.0f
0x1404822F8 = 50000.0f
0x14063A048 = 1.0f
```

## Antialiasing

`Antialiasing` é lido do JSON e armazenado na global `0x1409C9455`, porém não foi encontrado getter/callsite correspondente na camada principal do jogo analisada.

Status correto:

```text
CONFIGURADO/PARSEADO: sim
USO NO GAMEPLAY/RENDERER: não comprovado nesta build
```

Não devemos assumir que MSAA/FXAA/TAA está ativo apenas porque a chave existe.

## Recomendação para nosso renderer

Manter configuração validada em uma única camada e fazer OpenGL 4.6+ e Vulkan consumirem o mesmo `GraphicsConfig`:

```cpp
struct GraphicsConfig
{
    bool  shadows;
    int   shadowMode;
    float shadowFarZ;
    bool  shadowRightHanded;
    int   shadowCascades;
    int   shadowResolution;
    float shadowPartitioning;
    int   shadowFilterSize;
    bool  shadowFilterAcrossCascades;
    bool  shadowBestCascadeSearch;
    bool  verticalSync;
};
```

Isso evita divergência de comportamento entre backends.
