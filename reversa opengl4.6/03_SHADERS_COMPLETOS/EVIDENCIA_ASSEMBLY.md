# Evidência da montagem exata dos shaders GL4.6

## Função principal

VA: `0x0046F040`

O assembly mostra a montagem equivalente a:

```text
sources[0] = 0x00B8CAE4   ; "#version 460 core\n"
sources[1] = stageDefine  ; define do renderer
sources[2] = shaderBody   ; corpo vertex/fragment

glShaderSource(shader, 3, sources, nullptr)
glCompileShader(shader)
```

Portanto a ordem confirmada é **version -> define -> body**.

## Pares vertex/fragment observados em `.data`

```text
0x00C6C448 -> 0x00B8F348  Renderer2D vertex
0x00C6C44C -> 0x00B8F550  Renderer2D fragment
0x00C6C450 -> 0x00B8FFB0  Sky vertex
0x00C6C454 -> 0x00B90168  Sky fragment
0x00C6C458 -> 0x00B903B0  Atmosphere vertex
0x00C6C45C -> 0x00B904E0  Atmosphere fragment
0x00C6C460 -> 0x00B90C88  Water vertex
0x00C6C464 -> 0x00B90EF0  Water fragment
0x00C6C468 -> 0x00B90C88  Cloth vertex (reuso)
0x00C6C46C -> 0x00B92568  Cloth fragment
0x00C6C490 -> 0x00B93A40  Terrain vertex
0x00C6C494 -> 0x00B93EF8  Terrain fragment
```

## Defines observados

```text
0x00B8F9F4  #define RENDERER2D_GL46 1
0x00B90240  #define RENDERER_SKY_GL46 1
0x00B90AA0  #define RENDERER_ATMOSPHERE_GL46 1
0x00B91230  #define RENDERER_WATER_GL46 1
0x00B9291C  #define RENDERER_CLOTH_GL46 1
0x00B98388  #define RENDERER_TERRAIN_GL46 1 + TERRAIN_WEATHER_FX 1
0x00B983C8  #define RENDERER_TERRAIN_GL46 1 + TERRAIN_WEATHER_FX 0
```

Isso permite reconstruir os programas GL4.6 exatamente como são entregues ao driver, embora os nomes de arquivo usados nesta pasta sejam nomes organizacionais da engenharia reversa.
