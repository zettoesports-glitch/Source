# reversa opengl4.6

Material de engenharia reversa estática do `Main.exe` fornecido para comparação com o branch `modernization`.

## Importante

Isto **não é a source original recuperada 1:1**. Um executável C++ otimizado perde nomes locais, macros, comentários, tipos e parte da organização original. Esta pasta separa três classes de material:

- **EXTRAÍDO**: bytes/texto presentes literalmente no binário, como shaders GLSL, paths de PDB/source, imports e strings.
- **RECONSTRUÍDO**: C++/pseudocódigo derivado do assembly com comportamento preservado onde foi possível confirmar.
- **INFERIDO**: nomes/estrutura criados para tornar a reconstrução legível; devem ser validados antes de portar ao cliente.

O `Main.exe` original **não é incluído** nesta pasta.

## Identidade do binário analisado

- SHA-256: `dbd38a803e0d7b26d0d8f5b8b693105da12838e66ab0b0e3c93a64ac9b768d92`
- MD5: `90b656d65e85c91d5280bb4888a3fdc9`
- Formato: PE32 / x86 / Windows GUI
- ImageBase: `0x00400000`
- Entry point VA: `0x00AFBCC3`
- Timestamp PE: `2026-09-08 09:09:32`
- Linker: MSVC 14.29
- PDB embutido: `C:\Genesys\x6 Plus\Src S6 Plus\SRC\Main5.2\Release_EX603\Main.pdb`

## Achados principais confirmados

1. Renderer selecionável por `OpenGlUseShader` no registro `SOFTWARE\Webzen\Mu2\Config`.
2. Contrato observado: `0 = OpenGL 2 legacy`, `1 = OpenGL 3.3 Compatibility`, `2 = OpenGL 4.6`.
3. Para modo 2, tenta contexto WGL 4.6; o perfil 4.6 pode ser Core ou Compatibility. Se necessário, há caminho 3.3 Compatibility.
4. `OPENGL32.dll`, `GLU32.dll` e `glew32.dll` são imports reais.
5. Recursos modernos observados: VAO/VBO, shader objects, UBO, SSBO capability, buffer storage, timer query, debug output, sampler objects, sync, instancing, multi-draw-indirect e compute-shader capability.
6. `RendererModel` contém GPU skinning com `u_Bones[200]`, Chrome modes, BlendMesh, fog, alpha cutoff, shadow e `FrameData` UBO.
7. Há subsistemas nomeados `Renderer2DGL46`, `RendererTerrainGL46`, `RendererClothGL46`, `RendererSkyGL46`, `RendererAtmosphereGL46` e `RendererWaterGL46`.
8. A build contém telemetria/otimizações nomeadas `G01..G07`, `H08` e `P01`.
9. O executável preserva diversos paths de source e shaders GLSL em texto, facilitando a reconstrução.

## Estrutura

- `docs/` — análise e mapa do renderer.
- `evidence/` — dados extraídos literalmente do PE.
- `disassembly/` — trechos x86 relevantes com endereço virtual.
- `shaders/extracted/` — GLSL recuperado literalmente do executável.
- `source_reconstructed/` — C++ reconstruído/skeleton para servir como referência de port.

## Regra para uso no projeto principal

Nada desta pasta deve entrar no build automaticamente. Primeiro comparar comportamento e tipos com a source do cliente no branch `modernization`, depois portar manualmente somente o que for validado.
