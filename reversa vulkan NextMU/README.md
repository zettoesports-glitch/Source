# Reversa Vulkan NextMU

Análise estática clean-room dos executáveis `NextMU.exe` (x86) e `NextMU(1).exe` (x64), separada da pasta `reversa opengl4.6`.

## Resultado principal

O NextMU possui suporte real a múltiplos backends através da **Diligent Engine**:

- Direct3D 11
- Direct3D 12
- OpenGL
- Vulkan

O Vulkan não está implementado diretamente dentro do EXE. O executável carrega dinamicamente o plugin da Diligent Engine e resolve a factory Vulkan em runtime.

### DLLs esperadas

- x86: `GraphicsEngineVk_32.dll`
- x64: `GraphicsEngineVk_64.dll`

Símbolo procurado:

```text
GetEngineFactoryVk
```

O mesmo padrão existe para:

```text
GraphicsEngineD3D11
GraphicsEngineD3D12
GraphicsEngineOpenGL
GraphicsEngineVk
```

## Evidência no executável

Strings confirmadas nos dois binários:

```text
GetEngineFactoryD3D11
GraphicsEngineD3D11
GetEngineFactoryD3D12
GraphicsEngineD3D12
GetEngineFactoryOpenGL
GraphicsEngineOpenGL
GetEngineFactoryVk
GraphicsEngineVk
```

A rotina de loader monta o nome da DLL com o formato:

```text
%s%s%s.dll
```

com sufixo `_32` no executável x86 e `_64` no executável x64, depois usa `LoadLibraryA` e `GetProcAddress`.

## Ordem de tentativa dos backends

Os dois executáveis contêm a sequência de device types:

```text
1, 5, 2, 3
```

Pelo switch de `MUGraphics::InitializeEngine`, os valores mapeados são:

```text
1 = Direct3D11
2 = Direct3D12
3 = OpenGL
5 = Vulkan
```

Logo a ordem automática observada é:

```text
Direct3D11 -> Vulkan -> Direct3D12 -> OpenGL
```

## Source path preservado

```text
D:\NextMU\client-cmake\client\game\src\mu_graphics.cpp
```

Também aparece o símbolo:

```text
MUGraphics::InitializeEngine
```

## Limite atual

Para reconstruir o backend Vulkan específico usado por esse cliente no nível baixo, precisamos analisar também:

```text
GraphicsEngineVk_32.dll
GraphicsEngineVk_64.dll
```

O código base da Diligent Engine é open source, então a arquitetura pública pode servir de referência, mas a versão exata e eventuais alterações feitas pelo projeto NextMU só podem ser confirmadas analisando as DLLs que acompanham essa build.
