# 1 — Loader e seleção de backend no NextMU

## Backends confirmados

O EXE contém e resolve dinamicamente:

```text
GetEngineFactoryD3D11 / GraphicsEngineD3D11
GetEngineFactoryD3D12 / GraphicsEngineD3D12
GetEngineFactoryOpenGL / GraphicsEngineOpenGL
GetEngineFactoryVk / GraphicsEngineVk
```

O nome final da DLL é montado pelo formato:

```text
%s%s%s.dll
```

No pacote real, as DLLs possuem sufixo de arquitetura/build:

```text
_32r
_64r
```

Exemplo Vulkan:

```text
GraphicsEngineVk_32r.dll
GraphicsEngineVk_64r.dll
```

O EXE usa `LoadLibraryA` e `GetProcAddress`.

## Função do cliente preservada em strings

```text
MUGraphics::InitializeEngine
MUGraphics::InitializeEngine::<lambda_1>::operator ()
```

Source path:

```text
D:\NextMU\client-cmake\client\game\src\mu_graphics.cpp
```

## Device types observados

```text
1 = Direct3D11
2 = Direct3D12
3 = OpenGL
5 = Vulkan
```

Sequência automática observada nos executáveis:

```text
1, 5, 2, 3
```

Portanto:

```text
D3D11 -> Vulkan -> D3D12 -> OpenGL
```

## Tratamento de falha

O EXE possui mensagens específicas para falha de cada backend:

```text
Unable to initialize Diligent Engine in Direct3D11 mode...
Unable to initialize Diligent Engine in Direct3D12 mode...
Unable to initialize Diligent Engine in OpenGL mode...
Unable to initialize Diligent Engine in Vulkan mode...
```

Isso confirma que disponibilidade de API/GPU/driver é tratada como condição recuperável, não como crash obrigatório.

## Arquitetura recomendada para nosso MU

```text
RendererPreference
      |
      v
BackendSelector
      |
      +--> Vulkan available? ---- no ---> fallback
      |
      v
Load backend module
      |
      v
Resolve factory
      |
      v
Create device + immediate context
      |
      v
Create swapchain
      |
      v
Initialize renderer resources
```

Para nosso projeto, vale manter a ideia de um backend interface comum, mas podemos escolher ordem diferente, por exemplo:

```text
Vulkan -> OpenGL 4.6 -> OpenGL 3.3/legacy
```

ou deixar a escolha explícita pelo usuário.

## Regra de estabilidade

Não acoplar gameplay ao backend. O código de jogo deve falar com uma camada de render abstrata; Vulkan/OpenGL implementam essa camada. Isso reduz o custo de futuros updates e permite fallback sem reescrever sistemas de personagem, terrain, UI e effects.
