# 32 — Shadow pipeline e shader variants

Status: **CONFIRMADO** no `x64/NextMU.exe`.

## Configuração consumida pelo shadow system

Getters:

```text
Shadows                    0x140052950
ShadowBestCascadeSearch    0x140052960
ShadowCascadesCount        0x140052970
ShadowFarZ                 0x140052980
ShadowFilterAcrossCascades 0x140052990
ShadowFilterSize           0x1400529A0
ShadowMode                 0x1400529B0
ShadowPartitioning         0x1400529C0
ShadowResolution           0x1400529D0
ShadowRightHanded          0x1400529E0
```

## Shader permutations

A criação de shaders em torno de `0x140084560` monta macros de compilação:

```text
SKELETON_TEXTURE_WIDTH  = 2048
SKELETON_TEXTURE_HEIGHT = 512
USE_SHADOW              = Shadows
SHADOW_MODE             = ShadowMode
SHADOW_FILTER_SIZE      = ShadowFilterSize
FILTER_ACROSS_CASCADES  = ShadowFilterAcrossCascades
BEST_CASCADE_SEARCH     = ShadowBestCascadeSearch
```

Portanto várias decisões de shadow são especializadas em **compile time** e não ficam como branches genéricos por pixel.

## Runtime shadow setup

A região `0x1400760B0+` consulta pelo menos:

```text
ShadowResolution
ShadowCascadesCount
ShadowMode
```

e configura o objeto/manager de sombras.

## Bias dependente da resolução

Em torno de `0x140077C3F` foi observado:

```text
resolution >= 2048 -> 0.0025
resolution >= 1024 -> 0.0050
else               -> 0.0075
```

Como o parser desta build limita `ShadowResolution` a no mínimo 1024, o terceiro caminho parece defensivo/legado.

## Outros parâmetros

A mesma região inicializa valores próximos de:

```text
RGB ~ 0.2
A/4th ~ 0.3
```

A semântica exata ainda não está comprovada; permanecem como campos brutos, não devem ser nomeados como cor/intensidade sem correlação adicional.

## Arquitetura recomendada para nosso MU

Separar:

```text
ShadowConfig
    -> ShadowVariantKey
        -> shader/program cache
            -> GL4.6 program ou Vulkan PSO
```

Exemplo de chave lógica:

```cpp
struct ShadowVariantKey
{
    bool enabled;
    uint8_t mode;
    uint8_t filterSize;
    bool filterAcrossCascades;
    bool bestCascadeSearch;
};
```

Isso permite pré-compilar/reutilizar apenas variantes realmente necessárias e evita branches desnecessários dentro do shader.
