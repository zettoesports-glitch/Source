# reversa opengl4.6

Golden reference clean-room do OpenGL do `Main.exe` analisado, isolada do build principal do branch `modernization`.

## Para o nosso projeto, use só estes 3 blocos

### 1. Como o OpenGL 4.6 funciona e como implementar na source

`01_OPENGL46_SOURCE/README.md`

Contém contexto/fallback, WGL, capabilities, FrameData UBO, shader manager, primitive stream, Model/BMD e ordem segura de implementação.

### 2. O que vale portar/melhorar no nosso MU

`02_MELHORIAS_PARA_MU/ROADMAP.md`

Separa **PORTAR AGORA**, **PORTAR DEPOIS** e **FUTURO 4.6+**. Inclui bone scratch, pose reuse, uniform cache, batching 2D, opaque sort, terrain command encoder, text cache, particles/joints, weather e candidatos futuros como persistent mapping/MDI/SSBO/compute somente após benchmark.

### 3. Shaders completos e precisos

`03_SHADERS_COMPLETOS/README.md`

Os shaders GL4.6 montados ficam em `shaders/complete/`. A evidência mostra que o EXE chama `glShaderSource` com 3 partes: `#version 460 core` + define do renderer + corpo GLSL. O manifesto guarda VAs de origem por programa.

## Estado real de completude

- **Arquitetura OpenGL 4.6 necessária para construir nosso renderer:** suficientemente fechada como referência de implementação.
- **Shaders GL4.6 principais (2D/Sky/Atmosphere/Water/Cloth/Terrain base+weather):** reconstruídos completos em forma compilável/driver-equivalente.
- **Model/BMD/Shadow/Compat/Lightpass/MapAtmosphere:** strings completas extraídas do PE e preservadas em `shaders/extracted/`.
- **Biblioteca clean-room C++17:** compila no ambiente de análise.
- **Teste real dos GLSL em driver NVIDIA/AMD/OpenGL 4.6:** deve ocorrer no nosso cliente por compile/link logs; o ambiente de análise não possui driver/compiler GLSL 4.6.
- **Source original do Main 1:1:** impossível recuperar de um PE otimizado; nomes locais, headers e partes de integração não sobrevivem integralmente à compilação.

## Princípio para o OpenGL 4.6+

Primeiro fidelidade e estabilidade; depois performance. Capability detectada não significa feature obrigatória. SSBO/compute/MDI entram somente se benchmark provar ganho e houver fallback.

O `Main.exe` original não é armazenado no repositório.
