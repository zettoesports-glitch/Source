# Evidence summary

Static analysis was performed without executing the binary.

## PE / debug

- PE32 Intel i386, ImageBase `0x00400000`, entry `0x00AFBCC3`.
- Five sections: `.text`, `.rdata`, `.data`, `.fptable`, `.rsrc`.
- CodeView RSDS points to `C:\Genesys\x6 Plus\Src S6 Plus\SRC\Main5.2\Release_EX603\Main.pdb` (age 7).
- Export table: `AmdPowerXpressRequestHighPerformance`, `GetFunctionOffset`, `GetSharedValue`, `NvOptimusEnablement`.

## String/xref scan

The indexed scan found 1,463 renderer/OpenGL-related printable strings and 1,763 direct x86 immediate references from `.text` to those strings. The filtered phase catalog has 52 named D/E/F/G/H/P stages; 54 direct phase-string xrefs were found.

Raw indexed/xref files are kept in the local reverse-analysis workspace; the GitHub package retains the phase-xref document and the source/disassembly evidence most useful for implementation.

## RTTI

1,211 MSVC RTTI type strings were found. Renderer-relevant types include:

- `RenderPoseHandle@ModelRenderer`
- `RenderInterface_GL2`
- `RenderInterface_GL3`
- Genesys render-interface wrappers for GL2/GL3
- `CPhysicsCloth`
- `CPhysicsClothMesh`

## High-value code addresses

- `0x00470C30`: renderer context selection/fallback.
- `0x00470D90`: WGL attribute-context helper.
- `0x0077E550` region: registry/config selection for `OpenGlUseShader`.
- `0x00473180` region: RendererModel initialization evidence already preserved under `disassembly/`.

See `PHASE_XREFS.md`, `OFFSETS_CONFIRMADOS.md` and the assembly snippets for auditable details.
