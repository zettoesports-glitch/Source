# 24 — ModelSettings 96-byte layout EXATO

Status: **SOURCE-CORRELATED / CONFIRMADO**.

O layout bruto recuperado por assembly foi ligado à estrutura `NModelSettings` da source histórica de 25/01/2024 e às escritas de `MUModelRenderer::RenderMesh`.

Tamanho:

```text
0x60 = 96 bytes
```

## Estrutura

```cpp
#pragma pack(4)
struct NModelSettings
{
    glm::vec4 LightPosition;      // 0x00, 16 B
    glm::vec4 BodyLight;          // 0x10, 16 B
    glm::vec4 BodyOrigin;         // 0x20, 16 B

    float BoneOffset;             // 0x30
    float NormalScale;            // 0x34
    float EnableLight;            // 0x38
    float AlphaTest;              // 0x3C

    float PremultiplyAlpha;       // 0x40
    float WorldTime;              // 0x44
    float ZTestRef;               // 0x48
    float Dummy1;                 // 0x4C

    glm::vec2 BlendTexCoord;      // 0x50, 8 B
    float Dummy2;                 // 0x58
    float Dummy3;                 // 0x5C
};
#pragma pack()

static_assert(sizeof(NModelSettings) == 96);
```

## Valores escritos no caminho normal de Model

```text
LightPosition     = terrain->GetLightPosition()
BodyLight         = bodyLight/config alpha, com opção PremultiplyLight
BodyOrigin        = vec4(config.BodyOrigin, 0)
BoneOffset        = float(config.BoneOffset)
NormalScale       = 0.0
EnableLight       = float(config.EnableLight)
AlphaTest         = settings->AlphaTest
PremultiplyAlpha  = flag calculada a partir do material/texture/blend state
WorldTime         = MUState::GetWorldTime()
ZTestRef          = -3000.0
Dummy1            = 0.0
BlendTexCoord     = vec2(0,0) neste caminho
```

`Dummy2/Dummy3` fazem o fechamento/padding da estrutura de 96 bytes nessa revisão.

## PremultiplyAlpha

Não é simplesmente copiado do JSON. O cliente combina:

```text
settings->PremultiplyAlpha
texture->HasAlpha()
SrcBlend
```

para decidir o valor enviado ao shader.

## Observação

O nome anterior deste arquivo continha `RAW_LAYOUT` porque, antes da correlação com a source histórica, só os offsets eram conhecidos. O nome físico foi mantido para preservar links/histórico, mas o conteúdo agora é o layout semântico exato.
