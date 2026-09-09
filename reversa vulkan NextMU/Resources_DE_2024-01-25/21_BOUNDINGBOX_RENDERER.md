# 21 — Bounding-box renderer moderno

Status: **CONFIRMADO por assembly x64**.

Existe um programa dedicado:

```text
boundingbox
```

Isso mostra que debug/selection geometry também passa pelo pipeline moderno, em vez de depender de immediate-mode legado.

## Recursos confirmados

### Uniform buffer

```text
Size      = 96 bytes
Bind      = BIND_UNIFORM_BUFFER
Usage     = USAGE_DYNAMIC
CPUAccess = CPU_ACCESS_WRITE
```

Global associado fica na região:

```text
0x1409CA730
```

### Index buffer

```text
Size = 72 bytes
Usage = default/static path
InitialData = presente
```

72 bytes fecham exatamente:

```text
36 indices × 2 bytes = 72 bytes
```

36 índices = 12 triângulos, exatamente uma caixa triangulada de 6 faces.

Portanto a interpretação fortemente suportada é:

```text
8 corners / box vertices
36 uint16 indices
12 triangles
```

## Por que isso importa

Esse renderer é uma boa referência de como tratar pequenas primitivas auxiliares no nosso GL4.6+/Vulkan:

- PSO próprio;
- geometria indexada estática;
- constants dinâmicos pequenos;
- sem `glBegin/glEnd`;
- sem criar índices a cada frame.

A mesma estratégia serve para selection boxes, debug bounds, targeting/GM overlays e ferramentas internas.
