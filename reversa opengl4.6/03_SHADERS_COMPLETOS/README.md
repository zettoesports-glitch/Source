# 3 — Shaders completos e precisos

## Resultado

Os programas GL4.6 principais foram reconstruídos na forma **exata que o driver recebe** quando o EXE monta:

```text
#version 460 core
+ define do renderer
+ corpo GLSL literal do Main.exe
```

A evidência de assembly mostra `glShaderSource(..., count=3, ...)`.

Os arquivos finais ficam em:

`shaders/complete/`

## Programas GL4.6 completos

- `Renderer2DGL46.vert.glsl`
- `Renderer2DGL46.frag.glsl`
- `RendererSkyGL46.vert.glsl`
- `RendererSkyGL46.frag.glsl`
- `RendererAtmosphereGL46.vert.glsl`
- `RendererAtmosphereGL46.frag.glsl`
- `RendererWaterGL46.vert.glsl`
- `RendererWaterGL46.frag.glsl`
- `RendererClothGL46.vert.glsl`
- `RendererClothGL46.frag.glsl`
- `RendererTerrainGL46.base.vert.glsl`
- `RendererTerrainGL46.base.frag.glsl`
- `RendererTerrainGL46.weather.vert.glsl`
- `RendererTerrainGL46.weather.frag.glsl`

## Model/BMD e passes auxiliares completos encontrados no PE

- `RendererModel.legacy.vert.glsl`
- `RendererModel.frame_ubo.vert.glsl`
- `RendererModel.legacy.frag.glsl`
- `RendererModel.material.frag.glsl`
- `RendererModel.shadow_legacy.vert.glsl`
- `RendererModel.shadow_frame_ubo.vert.glsl`
- `RendererModel.shadow.frag.glsl`
- `Terrain.lightpass.vert.glsl`
- `Terrain.lightpass.frag.glsl`
- `CompatPass_A/B/C` vertex + fragment
- `MapAtmosphere.fullscreen.vert.glsl`
- `MapAtmosphere.mist.frag.glsl`

## Precisão

`COMPLETE_SHADER_MANIFEST.json` guarda, por arquivo:

- estágio/renderizador;
- VA do `#version`;
- VA do define;
- VA do corpo;
- SHA-256;
- tamanho final.

Exemplo de montagem comprovada:

```text
version VA 0x00B8CAE4 -> #version 460 core
Renderer2D body      -> 0x00B8F348 / 0x00B8F550
Sky body             -> 0x00B8FFB0 / 0x00B90168
Atmosphere body      -> 0x00B903B0 / 0x00B904E0
Water body           -> 0x00B90C88 / 0x00B90EF0
Cloth body           -> 0x00B90C88 / 0x00B92568
Terrain body         -> 0x00B93A40 / 0x00B93EF8
```

O vertex body de Water e Cloth é literalmente reutilizado; o define muda o renderer.

Terrain usa o mesmo corpo com duas compilações:

```glsl
#define RENDERER_TERRAIN_GL46 1
#define TERRAIN_WEATHER_FX 0
```

ou

```glsl
#define RENDERER_TERRAIN_GL46 1
#define TERRAIN_WEATHER_FX 1
```

## Validação feita

Todos os arquivos em `shaders/complete/` foram verificados para:

- presença de `#version`;
- presença de `main()`;
- balanceamento de `{}`;
- ausência de fechamento truncado evidente;
- hash registrado no manifesto.

A biblioteca C++ clean-room que monta/compila esses shaders também compila em C++17.

### Limite de validação

O ambiente de análise não possui `glslangValidator`/driver OpenGL 4.6 para fazer compile real offline. Portanto a validação atual é **estrutural + evidência binária exata**, não um teste de driver NVIDIA/AMD. Quando portarmos para o nosso cliente, o shader manager deve registrar compile/link logs e transformar esse teste em validação real automaticamente.

## Regra para nosso projeto

Não editar estes arquivos de referência. Para melhorias futuras, copiar para o renderer principal e manter:

1. `reference/` — versão fiel ao Main analisado;
2. `production/` — nossa versão corrigida/melhorada;
3. diff documentado entre as duas.

Isso permite atualizar o OpenGL 4.6+ sem perder a referência estável.
