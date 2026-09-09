# 17 — Janela, resolução, VSync e Resize

Status: **CONFIRMADO onde indicado; ausência de Resize é evidência negativa da build analisada**.

## Configuração de resolução

Strings confirmadas:

```text
WindowWidth
WindowHeight
WindowMode
```

Globals:

```text
0x1406A7028 = largura configurada
0x1406A702C = altura configurada
```

Accessors:

```text
0x140052A10 -> width
0x140052A00 -> height
```

O loader valida mínimo de:

```text
640 x 480
```

Se uma dimensão cai abaixo disso, força `640x480`.

## WindowMode

O parser lê `WindowMode`. Quando o caminho configurado pede resolução de desktop, chama `SDL_GetDesktopDisplayMode(0, ...)` e copia `w/h` retornados para os globals de resolução.

Não atribuir ainda um rótulo semântico definitivo (`fullscreen=true/false`) ao valor do JSON: nesta build ele influencia a escolha da resolução, mas o uso de fullscreen real não foi encontrado.

## Criação da janela exata

Função: `0x1400539F0`.

Chamada reconstruída:

```cpp
SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WindowWidth,
    WindowHeight,
    SDL_WINDOW_SHOWN
);
```

Evidência assembly:

```text
x = y = 0x2FFF0000  -> SDL_WINDOWPOS_CENTERED
flags = 0x00000004  -> SDL_WINDOW_SHOWN
```

A janela NÃO é criada com `SDL_WINDOW_RESIZABLE`, `SDL_WINDOW_FULLSCREEN` ou `SDL_WINDOW_FULLSCREEN_DESKTOP` nessa build.

## Imports SDL relevantes

Presentes:

```text
SDL_CreateWindow
SDL_DestroyWindow
SDL_GetDesktopDisplayMode
SDL_GetWindowWMInfo
SDL_PollEvent
SDL_RestoreWindow
```

Ausentes nos imports do EXE:

```text
SDL_SetWindowSize
SDL_SetWindowFullscreen
SDL_SetWindowBordered
```

## Swapchain Resize

`ISwapChain::Resize` da Diligent v2.5.4 é vtable `+0x30`.

Foi feita busca em:

- callers diretos de `GetSwapChain()` (`0x140055330`);
- referências diretas ao global `ISwapChain*` (`0x1409C9490`);
- família de init/destruction `0x140055350..0x140056150`.

**Até este ponto não existe caller confirmado de `ISwapChain::Resize` nessa build.**

Interpretação mais segura: a build de 25/01/2024 opera com janela de tamanho definido na inicialização e swapchain criada uma vez para essa dimensão. Não afirmar que outras versões do NextMU não tenham resize.

## VSync / Present

O frame principal passa um `syncInterval` 0/1 para `ISwapChain::Present()`.

Existe ainda o helper de loading/upload `0x140055290`, que chama `Present(0)` somente em backends explícitos quando há trabalho acumulado.

## Recomendação para nosso MU

Não copiar a limitação de janela fixa. Para nosso renderer:

1. usar o bootstrap estável do NextMU;
2. adicionar `SDL_WINDOW_RESIZABLE` opcional;
3. tratar resize de forma explícita;
4. aguardar GPU/invalidar attachments quando necessário;
5. chamar `SwapChain::Resize(width,height)` ou recriar swapchain no backend próprio;
6. recriar somente resources dependentes de resolução;
7. manter PSOs e resources independentes de resolução em cache.
