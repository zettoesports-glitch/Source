# Validação dos shaders de referência

## O que foi validado

- todos os programas GL4.6 publicados possuem `#version 460 core`;
- cada arquivo possui `main()` e blocos `{}` balanceados;
- pares vertex/fragment foram associados a partir dos ponteiros reais em `.data`;
- a montagem `version + define + body` é comprovada por `glShaderSource(..., 3, ...)` no assembly;
- o `GL46ShaderProgram` clean-room compila como parte da biblioteca C++17 de referência;
- Model/BMD, Shadow, Compat, Lightpass e MapAtmosphere permanecem preservados como strings completas em `shaders/extracted/`.

## O que ainda precisa ser validado no nosso cliente

O ambiente de análise não possui driver OpenGL 4.6 nem `glslangValidator`. Portanto a validação final deve acontecer no MuMain com:

1. `glCompileShader` + `GL_COMPILE_STATUS`;
2. log completo de `glGetShaderInfoLog`;
3. `glLinkProgram` + `GL_LINK_STATUS`;
4. log completo de `glGetProgramInfoLog`;
5. KHR_debug habilitado em builds de desenvolvimento;
6. teste NVIDIA + AMD + Intel quando possível;
7. fallback automático para 3.3 Compatibility se a criação 4.6 falhar.

Essa validação de driver é a etapa correta antes de mover um shader de `reference` para `production`.
