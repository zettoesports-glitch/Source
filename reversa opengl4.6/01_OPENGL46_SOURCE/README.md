# 1 — OpenGL 4.6: como funciona e como implementar na source

Este é o documento principal para portar a arquitetura do `Main.exe` para o MuMain do branch `modernization`.

## Estado de completude

### Confirmado / fechado para referência

- Contrato `OpenGlUseShader`: `0=OpenGL2`, `1=OpenGL3.3 Compatibility`, `2=OpenGL4.6`.
- Seleção e fallback de contexto: GL4.6 -> GL3.3 Compatibility -> legado.
- Criação moderna via `wglCreateContextAttribsARB`.
- Montagem exata dos shaders GL4.6: `#version 460 core` + `#define` do renderer + corpo GLSL, enviados em **3 strings** a `glShaderSource`.
- Capability layer e separação entre "hardware suporta" e "renderer realmente usa".
- `FrameData` UBO/binding 0 usado nos renderers modernos.
- Primitive stream moderno com VAO/VBO/IBO para substituir immediate mode.
- Model/BMD com GPU skinning, `u_Bones[200]`, material/shadow, Chrome/BlendMesh/fog/alpha.
- Estrutura dos renderers 2D, Sky, Atmosphere, Water, Cloth e Terrain.
- Auditoria para impedir chamadas fixed-function no caminho Core.
- Compilador/linker clean-room de shaders equivalente ao observado no EXE.

### Ainda não é "source original 1:1"

- nomes locais e classes originais que o compilador removeu;
- lifetime exato de cada objeto GL em todos os subsistemas;
- todas as funções de gameplay que chamam o renderer;
- todos os detalhes de resize/fullscreen/recovery em todas as máquinas;
- ordem exata de todos os draws do cliente inteiro.

Portanto: **a arquitetura OpenGL 4.6 necessária para construir o nosso renderer está suficientemente fechada; o cliente inteiro não foi decompilado 1:1.**

## Fluxo que devemos usar no nosso MuMain

```text
Config
  |
  v
RendererSelection
  |-- 0 -> Legacy GL2
  |-- 1 -> GL3.3 Compatibility
  `-- 2 -> GL4.6 (preferencial)
             |
             v
      Create bootstrap context
             |
             v
      load wglCreateContextAttribsARB
             |
             v
      try GL 4.6 Core/Compatibility
             |
        falhou?
             v
      try GL 3.3 Compatibility
             |
             v
        Load capabilities
             |
             v
        Create FrameData UBO
             |
             v
        Compile programs
             |
     +-------+--------+---------+-------+
     |       |        |         |       |
   Model    2D     Terrain    Water   Effects...
```

## Ordem de implementação recomendada

1. **Renderer bootstrap e fallback** — nunca iniciar GL4.6 sem fallback seguro.
2. **KHR_debug / logs de shader e contexto** — erro deve ser diagnosticável.
3. **FrameData UBO** — câmera/projeção/viewport/time/fog em um contrato comum.
4. **State cache** — programa, VAO, textura, blend, depth, cull e sampler só mudam quando necessário.
5. **GLCorePrimitiveStream** — substituir glBegin/glEnd e client arrays.
6. **RendererModel/BMD** — primeiro Hero, depois objetos remotos/NPC/monster/bots com isolamento de estado.
7. **2D batcher** — sprites/chat/HP bars/texto.
8. **Terrain command encoder/batches**.
9. **Water/Sky/Atmosphere/Cloth/Effects**.
10. **LegacyReachabilityAudit** — caminho GL4.6 Core deve ficar sem fixed-function alcançável.

## Montagem exata dos shaders GL4.6

O assembly em torno de `0x0046F040` mostra o equivalente a:

```cpp
const char* parts[3] = {
    "#version 460 core\n",
    rendererDefine,
    shaderBody
};

glShaderSource(shader, 3, parts, nullptr);
glCompileShader(shader);
```

Isso foi reconstruído em:

- `source_reconstructed/include/GL46ShaderProgram.h`
- `source_reconstructed/core/GL46ShaderProgram.cpp`

## Contexto e fallback

Offsets de referência confirmados:

- seletor/contexto: `0x00470C30`
- helper WGL: `0x00470D90`

Contrato recomendado para nosso código:

```cpp
enum class RendererMode {
    LegacyGL2 = 0,
    GL33Compatibility = 1,
    GL46 = 2
};

bool StartRenderer(RendererMode wanted) {
    if (wanted == RendererMode::GL46) {
        if (TryCreate46()) return true;
        if (TryCreate33Compatibility()) return true;
        return StartLegacy();
    }
    if (wanted == RendererMode::GL33Compatibility)
        return TryCreate33Compatibility() || StartLegacy();
    return StartLegacy();
}
```

## Regra de estabilidade

Nenhuma feature 4.6 deve ser usada só porque aparece como capability. Exemplo: o EXE detecta SSBO/compute/MDI, mas o Model/BMD confirmado usa `u_Bones[200]` e FrameData UBO. Só ativar recursos avançados após benchmark e fallback.

## Arquivos de referência

- `source_reconstructed/core/RenderDevice3D.cpp`
- `source_reconstructed/core/RendererSelection.cpp`
- `source_reconstructed/core/GL46Capabilities.cpp`
- `source_reconstructed/core/GL46ShaderProgram.cpp`
- `source_reconstructed/core/GLCorePrimitiveStream.cpp`
- `source_reconstructed/core/LegacyReachabilityAudit.cpp`
- `source_reconstructed/core/RendererModel.cpp`
