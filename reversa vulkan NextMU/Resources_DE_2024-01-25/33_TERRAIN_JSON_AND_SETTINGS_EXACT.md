# 33 — terrain.json e TerrainSettings

Status: **CONFIRMADO** para nomes de campos, offsets observados e tamanho do uniform; semântica física de alguns floats continua RECONSTRUÍDA.

## Campos encontrados no loader

O loader do Terrain (`0x14007BF80..0x14007E2xx`) lê diretamente:

```text
program
grass_program
height_multiplier
heightmap
nav_mesh
lightmap
light_position
textures
grass_textures
mappings
attributes
models
objects
path
```

Há ainda configuração dinâmica de movimento:

```text
mod
mul
wind
scale
```

Erros específicos preservados no binário confirmam essas etapas:

```text
terrain.json missing ({})
terrain program not found ({}, {})
terrain grass program not found ({}, {})
failed to load heightmap ({})
failed to generate normal ({})
invalid terrain light ({})
failed to load lightmap ({})
invalid terrain light position ({})
terrain.json textures missing ({})
failed to load textures ({})
failed to load mappings ({})
failed to load attributes ({})
failed to load terrain model ({})
failed to load terrain objects ({})
```

## Program IDs

O loader resolve IDs de programa e armazena variantes:

```text
terrain program -> +0x20 / +0x22
grass program   -> +0x24 / +0x26
```

O render seleciona A/B conforme o modo de cena/shadow.

## Dados dinâmicos `mod/mul/wind`

No carregamento de configuração observado em `0x1400513A8..0x14005155D`:

```text
mod -> object +0xD0
mul -> object +0xD4

wind.scale -> object +0xD8
wind.mod   -> object +0xDC
wind.mul   -> object +0xE0
```

A nomenclatura `wind.*` acima deriva diretamente da entrada JSON `wind` seguida das chaves `scale`, `mod` e `mul`.

## TerrainSettings: 16 bytes

O buffer ligado como `TerrainSettings` é atualizado por `0x140048CC0`.

O código usa:

```text
IDeviceContext::MapBuffer   (+0x158)
MAP_WRITE
MAP_FLAG_DISCARD
... escreve 16 bytes ...
IDeviceContext::UnmapBuffer (+0x160)
```

Logo o uniform dinâmico é exatamente:

```cpp
struct TerrainSettingsRaw
{
    float x;
    float y;
    float z;
    float w;
}; // 16 bytes
```

O writer copia `object +0x138 .. +0x147` para o buffer.

## Atualização observada

Antes do upload, o código calcula dois valores periódicos a partir do tempo e dos pares `mod/mul`:

```cpp
phase0 = periodic(time, object.mod)      * object.mul;
phase1 = periodic(time, object.wind.mod) * object.wind.mul;
```

E grava aproximadamente:

```text
+0x138 phase0
+0x13C 10.0f
+0x140 phase1
+0x144 0.0f
```

A função matemática intermediária é equivalente a uma redução periódica/fmod-like; o nome exato da função original não é recuperável pelo binário.

`wind.scale` é carregado em `+0xD8`, mas sua utilização completa ainda deve ser rastreada antes de atribuir semântica definitiva a todos os componentes do shader.

## Arquitetura sugerida para nosso renderer

Podemos manter o contrato explícito:

```cpp
struct TerrainAnimationSettings
{
    float basePhase;
    float baseScaleOrConstant;
    float windPhase;
    float reserved;
};
```

mas os nomes acima são RECONSTRUÍDOS; para uma implementação fiel é melhor preservar primeiro os offsets/valores observados e renomear quando os shaders do Terrain forem fechados.
