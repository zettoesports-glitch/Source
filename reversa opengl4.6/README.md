# reversa opengl4.6

Pacote de engenharia reversa **clean-room** do subsistema OpenGL do `Main.exe` fornecido pelo usuário. Pasta isolada do build do cliente principal; serve como golden reference para o branch `modernization`.

## Confiança
- **EXTRACTED/CONFIRMADO**: literal em PE, import/export, string, RTTI, shader ou assembly.
- **RECONSTRUCTED**: comportamento equivalente derivado de assembly/xrefs/telemetria.
- **INFERRED**: estrutura limpa criada quando nomes/tipos originais não sobreviveram.

## Resumo confirmado
- PE32 x86, ImageBase `0x00400000`, entry `0x00AFBCC3`, build PE 08/09/2026 09:09:32.
- OpenGL 2 legacy / OpenGL 3.3 Compatibility / OpenGL 4.6 via `OpenGlUseShader=0/1/2`.
- `wglCreateContextAttribsARB`; seletor `0x00470C30`, helper `0x00470D90`.
- Capabilities GL4, DSA/buffer-storage/debug/UBO/SSBO/compute detectados; uso é documentado separadamente de mera capacidade.
- Model/BMD: `u_Bones[200]`, FrameData UBO, Chrome 1..10, BlendMesh, fog, alpha, shadow, pose reuse, queue e opaque sort conservador.
- 2D/UI, sky, atmosphere, water, terrain/weather, cloth/effects/joints possuem fases e telemetria próprias.
- Exports `NvOptimusEnablement` e `AmdPowerXpressRequestHighPerformance`.
- 1.763 xrefs diretos de `.text` para strings relevantes catalogados; 54 xrefs diretos às fases nomeadas.

## Estrutura
- `docs/`: arquitetura, fases, offsets, deep dives e xrefs.
- `evidence/`: imports/exports, strings indexadas, RTTI, paths, xrefs.
- `disassembly/`: trechos x86 selecionados.
- `shaders/extracted/`: blocos que começam em `#version`.
- `shaders/extracted_gl46/`: fragmentos adicionais de sky/atmosphere/water/cloth/terrain preservados por offset.
- `source_reconstructed/`: biblioteca C++17 clean-room que representa a arquitetura recuperada.

## Validação
A árvore `source_reconstructed/` foi validada como biblioteca C++17 independente. Isso valida a consistência da reconstrução clean-room; **não significa** que ela seja a source original do executável.

## Limite honesto
Um PE compilado não permite recuperar a source original 1:1: nomes locais, headers, macros, comentários perdidos e milhares de funções de gameplay não podem ser recriados exatamente. Este pacote busca esgotar o **subsistema OpenGL 4.6 recuperável** e manter toda inferência explicitamente marcada.

O `Main.exe` original não é armazenado no repositório.
