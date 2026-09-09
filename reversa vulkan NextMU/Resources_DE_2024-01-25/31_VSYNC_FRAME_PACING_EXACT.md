# 31 — VSync e frame pacing exatos

Status: **CONFIRMADO** no `x64/NextMU.exe`.

## VerticalSync

Getter:

```text
0x1400529F0
```

O valor é consumido no frame principal em torno de `0x14002E79C` e vai diretamente para o sync interval do swapchain:

```text
VerticalSync = false -> Present(0)
VerticalSync = true  -> Present(1)
```

Há um segundo caller relevante em torno de `0x1400568E1`.

## Software frame limiter

Quando:

```text
VerticalSync == false
```

e a global de frame limiter em `0x1409C94E8` está habilitada, o cliente executa espera ativa/temporizada para atingir aproximadamente:

```text
16.666666666666668 ms / frame
≈ 60 FPS
```

Constantes observadas:

```text
0x1404829D0 = 16.666666666666668
0x1404829D8 = 1000000.0
```

A segunda constante é compatível com conversão para microssegundos.

## Política resultante

```text
VSync ON
  -> Present(1)
  -> sem software wait adicional

VSync OFF + limiter ON
  -> Present(0)
  -> software pacing para ~60 FPS

VSync OFF + limiter OFF
  -> Present(0)
  -> frame livre
```

## Implicação para OpenGL 4.6+ / Vulkan

O frontend pode expor uma política comum:

```text
VSync
FrameLimit
TargetFrameTime
```

Backend Vulkan:
- escolher present mode compatível;
- não duplicar espera se o compositor/driver já sincroniza.

Backend OpenGL:
- `wglSwapIntervalEXT(1/0)`;
- usar o mesmo frame pacer apenas quando necessário.

Para estabilidade, o frame limiter deve ser separado de VSync em nossa implementação futura.
