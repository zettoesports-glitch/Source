# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências,
> para nunca se perder entre sessões. Toda sessão acrescenta uma entrada aqui.
> Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md` · Dependências: `RENDER_DEPENDENCY_MAP.md`

---

## 2026-09-07 — Sessão 2 (FASE 3 concluída)

### FASE 3 — MAPA DE DEPENDÊNCIAS

- [x] `RENDER_DEPENDENCY_MAP.md` criado na branch `modernization`.
- [x] Pipeline atual documentado: Winmain → CoreGLCompat / CShaderGL / OGL330.
- [x] Dependências de `CoreGLCompat`, `CShaderGL`, `New_ModelBMD`, `New_RenderBMD`, `BoneManager` e `ZzzBMD` mapeadas.
- [x] Fluxo BMD → VAO/VBO → shader → draw documentado.
- [x] Fluxo `OBJECT::BoneTransform` → bone palette → `u_Bones` documentado.
- [x] Estados `RENDER_*` e dependências de blend/depth/texture/shader/VAO identificados para futura `PipelineState`.
- [x] Censo dos 61 `glBegin` e áreas consumidoras incorporado ao mapa.
- [x] Ordem segura de migração definida, preservando `CoreGLCompat` como fallback.
- [x] Vulkan mantido fora do caminho crítico até a RHI estar estável.

**Commit FASE 3:** `51bf424e9117d3408783d4a30e0b015c7a239ddc`

### Decisão para a FASE 4

Não reescrever o renderer atual. A fundação será adicionada em paralelo:

```text
RenderTypes
   ↓
RenderConfig / RenderStats
   ↓
IRenderDevice
   ↓
OpenGLRenderDevice
   ↓
BindState / ShaderManager / UBO
```

`CoreGLCompat` continua sendo o caminho de compatibilidade durante a migração.

---

## 2026-09-06 — Sessão 1 (FASE 0 concluída + FASE 2 iniciada)

### O que foi feito

**FASE 0 — SEGURANÇA**
- [x] `git init` no `C:\MUVULKAN` (branch `main`), commit baseline `3b3abe2`
      com 18.425 arquivos: Cliente, Source, MuServer, Launcher, shaders e o plano.
- [x] `.gitignore`: exclui `MuGames.rar` (616 MB — backup redundante do próprio
      workspace), zips duplicados dos repos de shader e caches `ipch/`, `.vs/`.
- [x] Branch `modernization` criada a partir de `main` (estratégia Git da seção 28).
- [x] **Build original verificado:** `MSBuild Main.sln /p:\"Configuration=Global Release;Platform=x86\"`
      compila **sem erros** (apenas warnings LNK4099 de PDB do cryptlib.lib).
      Saída: `Source/Main/Global Release/Main.exe`. VS instalado:
      `C:\Program Files\Microsoft Visual Studio\18\Community` (toolset v143).
- [x] **Baseline de GPU/driver registrada** (de `Cliente/opengl.log`):
      NVIDIA GeForce RTX 2060 · OpenGL 4.6 Core (requested 4.6) · GLSL 4.60 · driver NVIDIA 610.88.
- [ ] FPS base — PENDENTE (exige rodar o cliente em jogo; medir no login e num mapa)
- [ ] Cliente original testado — PENDENTE (rodar `Cliente/Main.exe` e validar)

**FASE 1 — Referências**
- [x] MuMain **Yesid** baixado pelo dono em `C:\MUVULKAN\yesid-bocanegra\MuMain-workspace`
      (1,7 GB, tem `.git` próprio). **Decisão:** NÃO versionar no nosso repo (`.gitignore`);
      usar como referência. Docs mais valiosos: `docs/architecture-rendering.md`,
      `docs/architecture-mumain.md`, `docs/adr/`.
- [ ] MuMain **Sven** — **não está no GitHub** (decisão do dono, 2026-09-06).
      Pendente até ser fornecido localmente. FASE 1 ajustada para avançar com
      Yesid + repos de shader.
- [x] Repos de shaders já presentes: `Shaders e winrar/opengl-main/OpenGL`
      (models, terrains, effects, shadows, cloth, fonts, interface, Shadow0-2) e
      `Shaders e winrar/vulkan-main/Shaders` (cloth, common, containers, effects,
      fonts, interface, models, shadows, terrains). Estrutura = a proposta no plano.

**FASE 2 — Auditoria do renderer**
- [x] `LUOIS_RENDERER_AUDIT.md` criado.
- [x] Contexto OpenGL 4.6 Core e emulação FFP via `CoreGLCompat` documentados.
- [x] OGL330/BMD e GPU skinning auditados.
- [x] 61 ocorrências reais de `glBegin` catalogadas.
- [x] `CShaderGL`/UBO e `CoreGLCompat`/batching/cache documentados.

**FASE 1 — Repositórios de shaders**
- [x] `SHADER_REPOS_SURVEY.md` criado.
- [x] OpenGL e Vulkan shaders levantados e relação com GlobalUBO/GPU skinning registrada.

### Decisões registradas
- `yesid-bocanegra/` fora do git do projeto (referência de terceiros, 1,7 GB).
- `MuGames.rar` e zips fora do git (redundantes).
- Arquivos da source em ISO-8859-1 (ex.: `CShaderGL.cpp`): converter com
  `iconv -f ISO-8859-1 -t UTF-8` antes de ler.
- Build de referência: `Global Release|x86` (a config que gera o cliente).

---

## Como buildar (referência rápida)

```bat
cd C:\MUVULKAN\Source\Main
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  Main.sln /p:"Configuration=Global Release;Platform=x86" /m /v:m
```

## Estado do git

- `main` — baseline imutável (commit `3b3abe2`)
- `modernization` — branch de trabalho atual
- Último commit da FASE 3: `51bf424e9117d3408783d4a30e0b015c7a239ddc`
