# 24 — ModelSettings 96-byte raw layout

Status: **CONFIRMADO NO NÍVEL DE OFFSETS**. Nomes semânticos permanecem deliberadamente neutros onde o shader/estrutura original ainda não foi ligado ao campo.

Tamanho:

```text
0x60 = 96 bytes
```

## Escritas observadas

```text
0x00..0x0F : float4/vector copiado de source +0xC0
0x10..0x1F : float4 computado durante material/light setup

0x20 : uint32/dword copiado de source +0x04
0x24 : float copiado de source +0x08
0x28 : float copiado de source +0x0C
0x2C : 0

0x30 : float convertido de source +0x00
0x34 : 0
0x38 : source byte +0x14 convertido para float
0x3C : uint32/dword de render/material state +0x48

0x40 : float flag 0.0/1.0 definido por branch
0x44 : float retornado por função 0x14007E420
0x48 : -3000.0f (0xC53B8000)
0x4C : 0

0x50..0x57 : zero
0x58..0x5F : padding/unused ou campos ainda sem escrita confirmada no caminho analisado
```

## Representação clean-room segura

Até a semântica ser comprovada, usar nomes neutros:

```cpp
struct ModelSettingsRaw
{
    float4 v0;          // 0x00
    float4 v1;          // 0x10

    uint32_t u20;       // 0x20
    float    f24;       // 0x24
    float    f28;       // 0x28
    uint32_t zero2C;    // 0x2C

    float    f30;       // 0x30
    float    zero34;    // 0x34
    float    f38;       // 0x38
    uint32_t u3C;       // 0x3C

    float    flag40;    // 0x40
    float    f44;       // 0x44
    float    constant48;// 0x48 = -3000
    float    zero4C;    // 0x4C

    uint64_t zero50;    // 0x50
    uint64_t tail58;    // 0x58
};
static_assert(sizeof(ModelSettingsRaw) == 96);
```

## Por que não nomear agora

Mesmo que alguns valores pareçam `BodyLight`, alpha, UV offset, fog etc., atribuir nome por aparência cria dívida técnica e pode causar um shader incompatível. O próximo passo é ligar cada offset ao shader/consumer e então promover os nomes para **CONFIRMADO**.
