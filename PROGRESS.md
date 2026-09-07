# PROGRESS.md — Diário da Modernização Luois 5.2

> **Propósito:** registrar tudo que foi feito, decisões tomadas e pendências,
> para nunca se perder entre sessões. Toda sessão acrescenta uma entrada aqui.
> Plano mestre: `MODERNIZATION_PLAN_V2.md` · Auditoria: `LUOIS_RENDERER_AUDIT.md`

---

## 2026-09-06 — Sessão 1 (FASE 0 concluída + FASE 2 iniciada)

### O que foi feito

**FASE 0 — SEGURANÇA**
- [x] `git init` no `C:\MUVULKAN` (branch `main`), commit baseline `3b3abe2`
      com 18.425 arquivos: Cliente, Source, MuServer, Launcher, shaders e o plano.
- [x] `.gitignore`: exclui `MuGames.rar` (616 MB — backup redundante do próprio
      workspace), zips duplicados dos repos de shader e caches `ipch/`, `.vs/`.
- [x] Branch `modernization` criada a partir de `main` (estratégia Git da seção 28).
- [x] **Build original verificado:** `MSBuild Main.sln /p:"Configuration=Global Release;Platform=x86"`
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

**FASE 2 — Auditoria do renderer (iniciada)**
- [x] `LUOIS_RENDERER_AUDIT.md` **v0.1** criado, cobrindo Winmain/contexto GL,
      CoreGLCompat, CShaderGL, MU_OpenGL, flag e censo glBegin. Descobertas-chave:
  1. O cliente **já roda OpenGL 4.6 Core** com a FFP 100% emulada via
     `CoreGLCompat` (macros interceptam ~57 funções legadas; 1 TU fala com a GL real).
  2. O caminho moderno OGL330 (CShaderGL + New_ModelBMD/New_RenderBMD) está
     **dormente**: o flag `jdk_shader_local330` não é definido em lugar nenhum
     (`#if` indefinido = 0 → código compilado fora).
  3. O ImmediateRenderer/BindState das FASES 4–5 **já existem como embrião**
     dentro do CoreGLCompat (batching, VBO streaming 8 MB, cache de estado,
     emulação de fog/alpha/texenv, ApplyFogUniforms).
  4. Censo `glBegin`: 61 ocorrências em 13 arquivos (maior: InGameUpdater 25,
     ZzzLodTerrain 9, ZzzEffectNoUse 8 — este último possivelmente código morto).
  5. `CShaderGL` já tem UBO (`MeshRenderState`) e construtores de model matrix
     a partir de matriz 3x4 de bone → semente do GPU Skinning (FASE 7).

**FASE 1 — Repositórios de shaders (levantado)**
- [x] Dono confirmou: os shaders OpenGL e Vulkan estão em `Shaders e winrar/`.
- [x] `SHADER_REPOS_SURVEY.md` criado. Resumo:
  - `opengl-main/opengl-main/OpenGL`: 176 .vs + 64 .ps, **GLSL decompilado via
    SPIRV-Cross** (fonte original é HLSL). Tem `GlobalConstants` (std140) com
    Game3D/Game2D/Shadow/luz/tempo → layout de referência do GlobalUBO.
  - `vulkan-main/vulkan-main/Shaders`: 45 .vs + 17 .ps + 8 .inc, **HLSL fonte
    legível**. `common/SkeletonHelpers.inc` = implementação de referência de
    **GPU skinning por textura de ossos** (`RequestBone`, 2 ossos/vértice,
    suporte a dual quaternion) — valida a seção 16 do plano.
  - Implicação: na FASE 7, usar os `.inc` HLSL como fonte da verdade.

### Decisões registradas
- `yesid-bocanegra/` fora do git do projeto (referência de terceiros, 1,7 GB).
- `MuGames.rar` e zips fora do git (redundantes).
- Arquivos da source em ISO-8859-1 (ex.: `CShaderGL.cpp`): converter com
  `iconv -f ISO-8859-1 -t UTF-8` antes de ler.
- Build de referência: `Global Release|x86` (a config que gera o cliente).

### Próximos passos (sessão seguinte)
1. Auditar `New_ModelBMD` + `New_RenderBMD` + `BoneManager` (o caminho dormente →
   plano de ativação do flag ou absorção sem guard).
2. Auditar `ZzzBMD` (formato/loader, 3.650 linhas).
3. Rodar o cliente e registrar FPS base (fechar FASE 0).
4. FASE 1: extrair lições de `yesid-bocanegra/.../docs/architecture-rendering.md`
   e cruzar `SkeletonHelpers.inc`/`GlobalConstants` com a Luois.

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
