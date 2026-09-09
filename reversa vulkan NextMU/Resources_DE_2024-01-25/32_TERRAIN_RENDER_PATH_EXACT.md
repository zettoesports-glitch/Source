# 32 — Terrain e Grass: render path exato

Status: **CONFIRMADO** por assembly do `NextMU.exe` x64.

## Função principal

```text
Terrain render: 0x1400516C0 .. 0x14005212D
```

O renderer usa o command buffer próprio do NextMU, não chama `IDeviceContext::Draw` diretamente nesse ponto.

## Recursos comuns do Terrain

Na primeira inicialização de um PSO (`wrapper +0x10 == 0`) são resolvidos via `IPipelineState::GetStaticVariableByName` e ligados com `IShaderResourceVariable::Set`:

```text
VS  cbCameraAttribs
VS  cbLightAttribs
PS  cbLightAttribs
VS  g_HeightTexture
VS  g_LightTexture
VS  g_NormalTexture
VS  g_MappingTexture
VS  g_UVTexture
VS  g_AttributesTexture
VS  TerrainSettings
PS  g_Textures
```

Os recursos do objeto Terrain usados no primeiro passe incluem:

```text
+0x28 Height texture
+0x30 Light texture
+0x38 Normal texture
+0x40 Mapping texture
+0x48 Attributes texture
+0x50 base texture collection
+0x60 base UV resource
+0x70 terrain vertex buffer
+0x78 TerrainSettings uniform buffer
+0x88 visible/range tree
+0x98 range table
```

Os nomes semânticos acima são confirmados pelos nomes de shader e pelos bindings. Os nomes dos campos C++ são reconstruídos.

## Shadow resources

Quando o modo de shadow está ativo, o renderer obtém/reutiliza um SRB no cache global `0x1409CA810` e liga um dos dois recursos:

```text
g_tex2DShadowMap
ou
g_tex2DFilterableShadowMap
```

A seleção depende do shadow mode retornado por `0x14007E750`.

## Submissão do passe base

O renderer enfileira:

```text
0x14009A900  SetVertexBuffers
0x14009A860  SetPipelineState
0x140099F70  CommitShaderResources
0x14009A020  Draw
```

O vertex buffer é `terrain +0x70`.

### Draw ranges

A árvore em `terrain +0x88` contém índices `uint16` em `node +0x1A`.

Cada índice acessa uma entrada de 8 bytes em `terrain +0x98`:

```cpp
struct TerrainDrawRange
{
    uint32_t begin;
    uint32_t end;
};
```

O draw é construído como:

```cpp
DrawAttribs draw{};
draw.NumVertices           = end - begin;
draw.Flags                 = DRAW_FLAG_VERIFY_ALL; // 7
draw.NumInstances          = 1;
draw.StartVertexLocation   = begin;
draw.FirstInstanceLocation = 0;
```

Portanto o Terrain é **não indexado** nesse caminho.

## Vertex format compacto

Conforme `28_INPUT_LAYOUTS_EXACT.md`:

```text
ATTRIB0 uint8x2 offset 0
ATTRIB1 uint8x2 offset 2
stride 4 bytes
```

O Terrain envia apenas 4 bytes de atributos por vértice e reconstrói dados espaciais com height/normal/UV/mapping textures no shader.

## Segundo passe: Grass — confirmado

O loader lê `grass_program` e grava o ID de programa nos campos `+0x24/+0x26` do objeto Terrain.

O render, após o passe base, verifica o recurso em `terrain +0x68` e, quando existe, seleciona exatamente:

```text
+0x24  grass program variant A
+0x26  grass program variant B
```

O passe Grass repete os bindings comuns de câmera/luz/height/light/normal/mapping/attributes/TerrainSettings, mas troca recursos específicos:

```text
VS g_UVTexture <- terrain +0x68
PS g_Textures  <- terrain +0x58
```

Em seguida executa a mesma sequência:

```text
SetVertexBuffers
SetPipelineState
CommitShaderResources
Draw(range)
```

usando o mesmo vertex buffer e a mesma tabela de ranges.

Isso prova um design eficiente:

```text
compact terrain geometry
        ↓
shared draw ranges
     ┌──┴───┐
     ↓      ↓
  Terrain  Grass
  PSO/SRB  PSO/SRB
```

## Implicação para nosso MU

Vale reproduzir:

1. Terrain em vertex format compacto;
2. chunks/ranges visíveis em uma lista organizada;
3. recursos globais ligados uma vez por PSO;
4. shadow SRB separado;
5. grass como segundo pipeline sobre a mesma geometria/ranges;
6. command buffer comum para GL4.6+/Vulkan.
