# FASE 7 — OpenGL BonesTexture Backend

## Estado

**CRIADO — ainda não integrado ao renderer**

O backend OpenGL envia o conteúdo de `SkeletonBuffer` para:

```
GL_TEXTURE_2D
GL_RGBA32F
width = 1024
filter = NEAREST
```

Isso corresponde ao GLSL de referência:

```glsl
uniform sampler2D BonesTexture;
uint boneX = texelIndex % 1024u;
uint boneY = texelIndex / 1024u;
return texelFetch(BonesTexture, ivec2(boneX, boneY), 0);
```

## Descoberta adicional

Os shaders GLSL atualmente presentes na árvore OpenGL usam o caminho
`USE_DUAL_QUATERNION` e consomem **2 texels por bone**.

O HLSL `SkeletonHelpers.inc` suporta:

- Matrix4x4: 4 texels/bone;
- DualQuaternion: 2 texels/bone.

Por isso `SkeletonBuffer` agora possui `StorageMode`.

**Não conectar Matrix4x4 ao shader OpenGL atual sem selecionar a variante correta.**

## Próximo passo

1. localizar/construir o encoder CPU de dual quaternion; ou
2. gerar/usar variante GLSL Matrix4x4 compatível com o frontend atual;
3. somente depois conectar o renderer runtime.
