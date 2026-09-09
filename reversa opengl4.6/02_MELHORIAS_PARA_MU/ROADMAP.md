# 2 — Melhorias do OpenGL 4.6 que valem para o nosso MU

Objetivo: construir um **OpenGL 4.6+ estável primeiro, rápido depois**, mantendo fidelidade visual do MU.

## PORTAR AGORA — estabilidade e correção

| Recurso | Por que vale | Prioridade |
|---|---|---|
| Contexto 4.6 + fallback 3.3/legado | evita cliente morto em GPU/driver problemático | P0 |
| KHR_debug + logs de compile/link | transforma tela preta/crash em diagnóstico | P0 |
| FrameData UBO | um contrato comum para view/projection/camera/viewport/time/fog | P0 |
| State cache | evita vazamento de estado e chamadas GL redundantes | P0 |
| Legacy reachability audit | impede glBegin/matrix stack/fixed fog no Core | P0 |
| Model material explícito | Chrome, BlendMesh, alpha, fog e blend deixam de depender do fixed-function | P0 |
| Isolamento por instância | evita Hero/NPC/bot/player reutilizando estado errado | P0 |
| Shader manager em 3 partes | reproduz exatamente a arquitetura observada | P0 |

## PORTAR LOGO DEPOIS — desempenho com baixo risco

### 1. Model bone scratch

Em vez de alocar vetor temporário por draw, usar storage reutilizável. O EXE registra `G03ModelBoneScratch`.

**Ganho:** menos heap/alloc/free durante cenas com muitos players/monsters.

### 2. Pose reuse

`G04ModelPoseReuse`: meshes que compartilham a mesma pose não precisam reconstruir/reler tudo.

**Ganho:** reduz CPU e uploads de bones.

### 3. Instance uniform cache

`H08ModelInstanceUniformCache`: não enviar uniform se o valor não mudou.

**Ganho:** menos chamadas de driver.

### 4. Command queue + opaque sort conservador

`G06/G07`: ordenar **somente opacos contíguos** e sem atravessar pose/programa/geometria/transparência.

**Ganho:** menos trocas de estado sem quebrar efeitos transparentes do MU.

### 5. Renderer2D batching

Agrupar sprites/chat/caixas/HP bars em buffers maiores.

**Ganho:** redução grande de draw calls da interface.

### 6. Text cache / HeadName reuse

Cachear superfícies de texto e nomes sobre personagens.

**Ganho:** menos rasterização/upload de texto repetido.

### 7. Terrain command encoder + state delta

Registrar comandos do terreno, mesclar quando seguro e aplicar apenas diferenças de estado.

**Ganho:** menos chamadas de depth/cull/blend/texture/fog e menos draw overhead.

### 8. Joint/particle batching

- deferred texture bind para partículas;
- direct Z rotation;
- adjacent joint batches;
- direct buffer upload.

**Ganho:** melhora cenas de skills/eventos com muitos efeitos.

## VISUAL / QUALIDADE que vale portar

- Sky GL4.6 dedicado;
- Atmosphere fullscreen triangle + noise/FBM;
- Water renderer dedicado;
- Terrain Weather FX;
- chão molhado;
- ripples de chuva;
- pegadas molhadas;
- turbidez/profundidade da água;
- frost/congelamento;
- lava com emissão própria;
- Cloth renderer moderno.

Esses recursos devem entrar **depois que Model/Terrain base estiverem estáveis**.

## FUTURO 4.6+ — melhorias nossas, não obrigatórias

O Main detecta capacidades como buffer storage, SSBO, compute e multi-draw-indirect. Isso abre espaço, mas não prova que o BMD usa essas técnicas.

### Candidatos para benchmark futuro

1. **Persistent mapped ring buffers** para 2D/particles/terrain dinâmico.
2. **DSA completo** (`glCreateBuffers`, `glNamedBufferSubData`, etc.) com fallback bind-based.
3. **MultiDrawIndirect** em geometria estática/terrain, só após medir.
4. **SSBO** para grandes conjuntos de instâncias ou dados auxiliares — não substituir `u_Bones[200]` sem benchmark.
5. **Compute shader** para partículas/efeitos futuros, não como requisito do renderer base.
6. **Program binary/cache de shaders** para reduzir tempo de startup em futuras versões, se os drivers alvo forem confiáveis.
7. **GPU timer queries + CSV metrics** para decidir otimizações com dados, não sensação.

## O que NÃO devemos fazer

- ativar todas as extensões 4.6 de uma vez;
- migrar bones para SSBO/compute só por ser "mais moderno";
- ordenar transparência junto com opacos;
- remover fallback 3.3 antes de testar GPUs antigas;
- misturar port do BMD com terrain/water/effects no mesmo passo;
- otimizar antes de eliminar vazamento de estado.

## Roadmap para nosso branch modernization

```text
Fase A  estabilidade do contexto + debug
Fase B  ModernBMD fiel (Hero)
Fase C  isolamento e rollout: remote player -> bot -> NPC -> monster
Fase D  bone scratch + pose reuse + uniform cache
Fase E  opaque command queue/sort
Fase F  2D batch + text cache
Fase G  terrain encoder/batch
Fase H  water/sky/atmosphere/weather
Fase I  particles/joints/cloth
Fase J  benchmarks 4.6+ (persistent mapping/MDI/SSBO/compute)
```
