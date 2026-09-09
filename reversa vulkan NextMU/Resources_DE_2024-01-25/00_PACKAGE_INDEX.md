# Resources_DE_2024-01-25 — índice da engenharia reversa

Esta pasta contém exclusivamente a análise estática clean-room do pacote `Resources_DE_2024-01-25.zip` fornecido pelo usuário.

## Conteúdo do ZIP

```text
x86/NextMU.exe
x86/SDL2.dll
x86/GraphicsEngineD3D11_32r.dll
x86/GraphicsEngineD3D12_32r.dll
x86/GraphicsEngineOpenGL_32r.dll
x86/GraphicsEngineVk_32r.dll

x64/NextMU.exe
x64/SDL2.dll
x64/GraphicsEngineD3D11_64r.dll
x64/GraphicsEngineD3D12_64r.dll
x64/GraphicsEngineOpenGL_64r.dll
x64/GraphicsEngineVk_64r.dll
```

## Resultado principal

O pacote implementa um cliente NextMU multi-backend usando Diligent Engine. Backends presentes:

```text
Direct3D 11
Direct3D 12
OpenGL
Vulkan
```

O Vulkan real fica nos plugins:

```text
x86/GraphicsEngineVk_32r.dll
x64/GraphicsEngineVk_64r.dll
```

Ambos exportam:

```text
GetEngineFactoryVk
```

O backend Vulkan carrega `vulkan-1.dll` dinamicamente e resolve a API Vulkan por `vkGetInstanceProcAddr`/`vkGetDeviceProcAddr`.

## Objetivo para nosso MU

Usar este pacote como referência para construir uma arquitetura de renderer moderna e estável, separando:

- seleção/fallback de backend;
- inicialização Vulkan;
- swapchain/queues/command buffers;
- memória e recursos;
- descriptors/pipelines/sync;
- shader cross-API para SPIR-V;
- telemetria/debug/validation;
- otimizações que realmente valem para um cliente MU.

## Regra de confiança

- **CONFIRMADO**: evidência literal em EXE/DLL/import/export/string/source path.
- **RECONSTRUÍDO**: fluxo equivalente derivado da evidência estática.
- **SUPORTADO PELO BACKEND**: a Diligent/Vulkan build contém a feature, mas isso não prova uso pelo jogo.
- **INFERIDO**: hipótese de integração ainda não provada.

Os binários originais não são copiados para o repositório.
