# 34 — Configuração gráfica e limites exatos

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64.

Loader principal observado em `0x140052Bxx..0x14005394F` lendo `config/game.json`.

## Chaves

```text
WindowWidth
WindowHeight
WindowMode
Shadows
ShadowMode
ShadowFarZ
ShadowRightHanded
ShadowCascadesCount
ShadowResolution
ShadowPartitioning
ShadowFilterSize
ShadowFilterAcrossCascades
ShadowBestCascadeSearch
Antialiasing
VerticalSync
MusicVolume
SoundVolume
```

## Janela

Globais:

```text
0x1406A7024 WindowMode (bool)
0x1406A7028 WindowWidth
0x1406A702C WindowHeight
```

Após leitura, o cliente valida:

```cpp
if (WindowWidth < 640 || WindowHeight < 480)
{
    WindowWidth  = 640;
    WindowHeight = 480;
}
```

Quando `WindowMode == false`, o código chama a rotina SDL equivalente a obter o modo do desktop; em sucesso, substitui Width/Height pela resolução do desktop. Em falha, muda `WindowMode` para `true`.

A semântica nominal exata de `true/false` (windowed/fullscreen) deve ser mantida como comportamento observado até fechar o ponto de `SDL_CreateWindow`/flags, mas a relação com resolução de desktop é CONFIRMADA.

## Shadows

```text
0x1409C9451 Shadows                  bool
0x1406A7030 ShadowMode               uint32
0x1406A7034 ShadowFarZ               float
0x1409C9452 ShadowRightHanded        bool
0x1406A7038 ShadowCascadesCount      uint32
0x1406A703C ShadowResolution         uint32
0x1406A7040 ShadowPartitioning       float
0x1406A7044 ShadowFilterSize         uint32
0x1409C9453 ShadowFilterAcrossCascades bool
0x1409C9454 ShadowBestCascadeSearch  bool
```

### Limites exatos

```text
ShadowMode:           clamp [1, 4]
ShadowFarZ:           clamp [1000.0, 50000.0]
ShadowCascadesCount:  clamp [1, 8]
ShadowResolution:     clamp [1024, 8192]
ShadowPartitioning:   clamp [0.0, 1.0]
ShadowFilterSize:     clamp [1, 4]
```

Constantes de `ShadowFarZ` estão em:

```text
0x1404822F4 = 1000.0f
0x1404822F8 = 50000.0f
```

`ShadowPartitioning` usa `max(0.0f, value)` seguido de `min(1.0f, value)`.

Booleans são convertidos pela rotina em `0x14003BE30`.

## AA / VSync

```text
0x1409C9455 Antialiasing bool
0x1409C9456 VerticalSync bool
```

Ambos são carregados como booleanos.

## Volume

```text
0x1406A7048 MusicVolume float
0x1406A704C SoundVolume float
```

O loader converte os valores JSON para `float`. Não foi observado clamp explícito nesse bloco.

## Tipos de conversão JSON reconhecidos

Pelos callsites:

```text
0x14003BC40 -> inteiro
0x14003BE30 -> bool
0x140019710 -> float
```

## Relevância para nosso renderer

Esses valores devem entrar em uma configuração backend-agnostic:

```cpp
struct GraphicsQualityConfig
{
    bool     shadows;
    uint32_t shadowMode;
    float    shadowFarZ;
    uint32_t cascades;
    uint32_t shadowResolution;
    float    partitioning;
    uint32_t filterSize;
    bool     filterAcrossCascades;
    bool     bestCascadeSearch;
    bool     antialiasing;
    bool     verticalSync;
};
```

OpenGL 4.6+ e Vulkan devem consumir a mesma configuração para preservar paridade visual.
