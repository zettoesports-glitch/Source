# Status objetivo

- Shaders GL4.6 de 2D/Sky/Atmosphere/Water/Cloth/Terrain: **RECONSTRUÍDOS COMPLETOS a partir das 3 partes que o EXE envia a glShaderSource**.
- Shaders Model/BMD/Shadow/Compat/Lightpass/MapAtmosphere listados no manifesto: **EXTRAÍDOS completos como strings embutidas**.
- Validação sintática por driver: **pendente para ocorrer dentro do nosso cliente**, pois o ambiente de análise não possui compilador GLSL/driver 4.6.
- Fidelidade source original: corpos e defines são baseados em bytes/ponteiros do PE; nomes de arquivo são nomes organizacionais da reconstrução, não necessariamente nomes originais em disco.
