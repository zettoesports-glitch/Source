# Status da reconstrução

## Recuperado com alta confiança

- identidade PE / imports / PDB / source paths sobreviventes;
- seleção `OpenGlUseShader`;
- criação de contexto WGL 4.6 e fallback 3.3 Compatibility;
- capabilities modernas e auditoria legacy;
- todos os 33 blocos `#version` encontrados no PE;
- shaders de `RendererModel` (model/material/shadow) em texto;
- shaders de `MapAtmosphere`;
- estruturas GLSL de FrameData, layout de vertex e 200 bones;
- nomes/telemetria de Renderer2D/Terrain/Cloth/Sky/Atmosphere/Water e fases G/H/P.

## Parcialmente reconstruído

- `RenderDevice3D.cpp`: lógica central de contexto, mas não todas as funções da classe original;
- `RendererModel.cpp`: arquitetura/estruturas e shaders; C++ completo de draw/queue ainda requer decompilação adicional;
- `GLCompatFixedState.cpp`: categorias e propósito confirmados; wrappers individuais ainda precisam ser mapeados;
- passes 14–21: GLSL extraído, associação de classe ainda não confirmada.

## Ainda não recuperado 1:1

A source integral do jogo. O `.text` possui ~7,5 MiB de código nativo e milhares de funções. Recuperar tudo exige decompilação e validação função por função. O objetivo desta pasta é preservar o que já foi realmente demonstrado e permitir continuar a reconstrução sem adivinhações.
