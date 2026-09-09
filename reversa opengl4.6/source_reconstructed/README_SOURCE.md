# Source reconstruída

Esta árvore é uma implementação **clean-room de referência** derivada do `Main.exe` analisado. Ela não é a source original e não é adicionada ao build principal.

## Níveis de confiança

- **EXTRACTED**: texto/valor literal no PE (shaders, paths, strings, imports).
- **RECONSTRUCTED**: comportamento reconstruído a partir de assembly/xrefs.
- **INFERRED**: API/estrutura limpa criada para representar evidência ainda incompleta.

O núcleo mais confiável hoje é: seleção de renderer, criação WGL 4.6→3.3, layout de FrameData, layout de ModelVertex, `u_Bones[200]`, Chrome/BlendMesh/fog/alpha/shadow GLSL e fases G01/G02/G03/G04/G06/G07/H08.
