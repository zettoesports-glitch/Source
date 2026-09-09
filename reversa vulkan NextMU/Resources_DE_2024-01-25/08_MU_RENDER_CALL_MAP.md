# 08 — MU render call map (NextMU x64)

Status: **CONFIRMADO onde há endereço/vtable; RECONSTRUÍDO onde o nome de helper original não sobreviveu.**

## Globais gráficos recuperados

| VA | Uso confirmado |
|---|---|
| `0x1409C9480` | engine factory / factory específica do backend |
| `0x1409C9488` | `IRenderDevice*` |
| `0x1409C9490` | `ISwapChain*` |
| `0x1409C94B0..B8` | storage de `IDeviceContext*` |
| `0x1409C946C` | `RENDER_DEVICE_TYPE` atual |
| `0x1409C9474` | índice do contexto atual |
| `0x1409C947C` | contador usado pelo Present controlado |

Accessors:

```text
0x1400552D0 -> IRenderDevice*
0x1400552F0 -> IDeviceContext* atual
0x140055330 -> ISwapChain*
0x1400552E0 -> tipo do device
0x140055340 -> incrementa contador de apresentação/trabalho
```

## Interfaces Diligent v2.5.4 usadas para nomear calls

O EXE grava `DILIGENT_API_VERSION=254000`. A source pública Diligent v2.5.4 define a mesma versão, permitindo mapear offsets virtuais sem adivinhação.

### IRenderDevice — início da vtable

```text
+0x20 CreateBuffer
+0x28 CreateShader
+0x30 CreateTexture
+0x38 CreateSampler
+0x40 CreateResourceMapping
+0x48 CreateGraphicsPipelineState
+0x50 CreateComputePipelineState
+0x58 CreateRayTracingPipelineState
```

### IDeviceContext — calls relevantes ao MU

```text
+0x30 SetPipelineState
+0x40 CommitShaderResources
+0x58 SetVertexBuffers
+0x68 SetIndexBuffer
+0x70 SetViewports
+0x78 SetScissorRects
+0x80 SetRenderTargets
+0xA8 Draw
+0xB0 DrawIndexed
+0xB8 DrawIndirect
+0xC0 DrawIndexedIndirect
+0xD8 DispatchCompute
+0xF8 ClearDepthStencil
+0x100 ClearRenderTarget
+0x148 UpdateBuffer
+0x158 MapBuffer
+0x160 UnmapBuffer
+0x168 UpdateTexture
+0x188 GenerateMips
+0x190 FinishFrame
+0x1A0 TransitionResourceStates
```

### ISwapChain

```text
+0x20 Present
+0x28 GetDesc
+0x30 Resize
+0x38 SetFullscreenMode
+0x40 SetWindowedMode
+0x50 GetCurrentBackBufferRTV
+0x58 GetDepthBufferDSV
```

## Sistemas MU localizados por RTTI

```text
NEntity::NRenderable
NEntity::NRenderState
NCharacters::PreRender(const NRenderSettings&)
NObjects::PreRender(const NRenderSettings&)
NParticles::Render
NJoints::Render
```

Endereços x64 confirmados:

```text
NCharacters::PreRender  0x1400678E0 - 0x140067E5F
NParticles::Render      0x140074170 - 0x1400749BA
NObjects::PreRender     0x1400A5A20 - 0x1400A6216
NJoints::Render         0x1400AA1B0 - 0x1400AA9FA
```

## Terrain

Carregamento/configuração:

```text
0x14004CDF0-0x14004DC6E  heightmap / ozb
0x14004DC70-0x14004E71C  lightmap / ozj / ozb
0x14004E720-0x14004F30E  mappings
0x14004F660-0x140051264  scaled / water / JSON
0x140051270-0x1400516B2  terrain.json / wind / scale / water
```

Render/resources:

```text
0x1400516E2-0x140052123
cbCameraAttribs
cbLightAttribs
g_HeightTexture
g_LightTexture
g_NormalTexture
g_MappingTexture
g_UVTexture
g_AttributesTexture
TerrainSettings
g_Textures
g_tex2DShadowMap
g_tex2DFilterableShadowMap
```

Program loader:

```text
0x14007BA60-0x14007E2F9
terrain.json
terrain program not found
_shadow
grass_program
terrain grass program not found
```

## Resources/UI

Resource manifest:

```text
data/
resources.json
attachments
shaders
textures
models
```

RmlUI:

```text
cbRmlAttribs
g_Texture
rmlui_color
rmlui_texture
linear
clamp
```

Em `0x1400AD8CC`, `IRenderDevice +0x20` confirma `CreateBuffer` para recursos da UI.

## Conclusão

A camada do jogo é centralizada em três objetos independentes da API:

```text
IRenderDevice  -> cria recursos e PSOs
IDeviceContext -> grava/envia comandos e transitions
ISwapChain     -> backbuffer/depth/present/resize
```

Esse é o modelo que vale reproduzir no nosso MU: Model/Terrain/UI não devem conhecer `VkDevice` diretamente.
