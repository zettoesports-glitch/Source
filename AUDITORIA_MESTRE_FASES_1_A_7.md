# AUDITORIA MESTRE — FASES 1 A 7 (OpenGL + Vulkan)

**Data:** 2026-09-07
**Branch auditada:** `modernization`
**Escopo:** plano mestre, progresso, renderer Luois, mapa de dependências, infraestrutura FASE 4–7 e contratos de shaders OpenGL/Vulkan.

---

## RESUMO EXECUTIVO

A auditoria encontrou uma base de modernização promissora, mas também uma divergência importante entre o plano original e o contrato real dos shaders.

### Resultado

| Área | Status |
|---|---|
| FASE 1 — investigação | 🟡 parcialmente comprovada |
| FASE 2 — auditoria Luois | 🟢 forte |
| FASE 3 — dependências | 🟢 forte |
| FASE 4 — fundação | 🔵 infraestrutura criada, integração parcial |
| FASE 5 — Immediate | 🔵 redundante em relação ao CoreGLCompat existente |
| FASE 6 — Core Profile | 🟢 já atendida por emulação; migração física pendente |
| FASE 7 — GPU Skinning | 🟠 arquitetura precisa convergir para contrato real |
| Preparação Vulkan | 🟡 conceitual, ainda sem backend físico |

**Regra principal após auditoria:** parar de adicionar infraestrutura paralela antes de consolidar os contratos existentes.

---

# FASE 1 — INVESTIGAÇÃO

## 🟡 ACHADO A1 — documentação de investigação incompleta

O plano exige `ARCHITECTURE_COMPARISON.md`, porém o arquivo não está presente na branch auditada.

### Impacto
A FASE 1 está marcada como concluída no progresso, mas o artefato final previsto no plano não existe.

### Ação
Criar matriz única:

```
Luois
MuMain Sven
MuMain Yesid
OpenGL shaders
Vulkan shaders
```

com:
- responsabilidade;
- código aproveitado;
- código apenas referência;
- incompatibilidades.

---

# FASE 2 — AUDITORIA LUOIS

## 🟢 ACHADO A2 — auditoria encontrou a realidade arquitetural

O cliente já usa OpenGL 4.6 Core com `CoreGLCompat` para emular FFP.

Isso muda o plano: o objetivo não é “fazer Core Profile”, mas reduzir gradualmente a emulação CPU.

## 🟡 ACHADO A3 — inconsistência histórica no relatório

O relatório descreve o pipeline BMD como “GPU skinning” enquanto o caminho atual ainda envia palette por uniform.

### Ação
Usar nomenclatura precisa:
- `GPU-ready BMD pipeline`
- `GPU skinning completo` somente após transporte + shader + validação visual.

---

# FASE 3 — DEPENDÊNCIAS

## 🟢 ACHADO A4 — mapa operacional útil

O fluxo:

```
Winmain
 -> CoreGLCompat
 -> CShaderGL
 -> OGL330
 -> New_ModelBMD
 -> New_RenderBMD
```

está bem identificado.

## 🟡 ACHADO A5 — mapa deve incluir contrato de shader

O mapa atual termina em OpenGL/VAO/shader, mas agora sabemos que existe contrato externo OpenGL/Vulkan para:
- locations;
- skeleton transport;
- instancing;
- shadows.

### Ação
Adicionar camada:

```
CPU frontend
   ↓
Vertex/Instance Contract
   ↓
Skeleton Contract
   ↓
OpenGL/Vulkan backend
```

---

# FASE 4 — FUNDAÇÃO

## 🔵 ACHADO A6 — RHI existe como fundação, não como sistema ativo

Arquivos Render/ foram criados, mas a auditoria não encontrou evidência de redirecionamento do renderer legado através de `IRenderDevice`.

### Risco
Criar abstrações desconectadas do runtime aumenta código morto.

### Ação
Não expandir interfaces RHI agora.
Próxima abstração só deve ser adicionada quando tiver um consumidor real.

## 🟠 ACHADO A7 — plano e referência Vulkan divergem em Uniforms

O plano propõe:
- GlobalUBO;
- SceneUBO;
- BoneUBO;
- ObjectUBO.

Os shaders de referência usam um contrato mais específico para skeleton:
`BonesTexture`.

### Ação
Manter Global/Scene como infraestrutura genérica.
Não tornar BoneUBO o contrato final.

---

# FASE 5 — IMMEDIATE RENDERER

## 🟠 ACHADO A8 — duplicação conceitual

O `CoreGLCompat.cpp` já implementa:
- acumulação;
- batching;
- conversão de primitivas;
- VBO streaming;
- VAO;
- cache parcial de estado.

Criar outro ImmediateRenderer paralelo pode duplicar a mesma responsabilidade.

### Ação recomendada
Transformar o ImmediateRenderer novo em fachada/adaptador do CoreGLCompat ou absorver gradualmente componentes do CoreGLCompat.

**Não manter dois sistemas de batching independentes.**

---

# FASE 6 — CORE PROFILE

## 🟢 ACHADO A9 — objetivo original já foi atingido tecnicamente

O contexto real é Core Profile.
As chamadas antigas são emuladas.

## 🟡 ACHADO A10 — progresso deve ser separado em dois níveis

```
Core Profile Runtime: CONCLUÍDO
Migração dos consumidores legados: PENDENTE
```

Não declarar a FASE 6 “100% concluída” enquanto os 61 pontos ainda dependem da camada de compatibilidade.

---

# FASE 7 — BMD / GPU SKINNING

## 🔴 ACHADO A11 — contrato experimental de 4 pesos não é o contrato dos shaders de referência

Infraestrutura experimental:

```
BoneIndices[4]
BoneWeights[4]
BoneUBO[200]
```

Contrato estudado:

```
location 3  uint2 Bones
location 12 uint BoneIndex
BonesTexture
RequestBone()
```

### Impacto
Não ativar a versão experimental globalmente.

### Ação
Classificar como EXPERIMENTAL e não integrar no runtime até reconciliação.

---

## 🔴 ACHADO A12 — modelos e shadows precisam compartilhar skeleton transport

O estudo mostrou que shadows reutilizam SkeletonHelpers.

### Risco
Migrar apenas ModelRenderer pode criar personagem animado correto com sombra incorreta.

### Ação
O `SkeletonBuffer` deve ser recurso compartilhado do frontend, não propriedade exclusiva de New_ModelBMD.

---

## 🟠 ACHADO A13 — BoneIndex por instância ainda não está integrado

O contrato usa um índice/base de skeleton por instância.

### Ação
Mapear:
- qual estrutura representa a instância;
- onde a palette é alocada;
- como offsets são calculados;
- como OpenGL e Vulkan recebem a mesma base.

---

## 🟢 ACHADO A14 — CPU fallback deve permanecer

O BoneManager fornece dados usados também fora do renderer.

### Regra
Não remover `OBJECT::BoneTransform` nem BoneManager.

O renderer deve consumir uma representação derivada da animação CPU.

---

# OPENGL × VULKAN

## 🟢 ACHADO A15 — contrato lógico compartilhado identificado

O caminho correto é:

```
Shared frontend data
      ↓
Shared shader semantics
      ↓
OpenGL backend / Vulkan backend
```

## 🟡 ACHADO A16 — não iniciar backend Vulkan físico antes de estabilizar OpenGL

Isso continua alinhado com o plano mestre.

Preparar a interface não significa implementar `VulkanRenderDevice` agora.

---

# PROBLEMAS DE PROCESSO

## 🔴 ACHADO A17 — progresso “concluído” não significa integração testada

Várias fases possuem infraestrutura criada, mas:
- novos .cpp ainda não estão no .vcxproj;
- não houve build após integração física;
- não houve teste visual do cliente.

### Novo significado obrigatório

| Estado | Significado |
|---|---|
| DOCUMENTADO | analisado |
| CRIADO | arquivo/código existe |
| INTEGRADO | chamado pelo runtime |
| COMPILADO | build passou |
| VALIDADO | teste real passou |

Não usar apenas “concluído”.

---

# PLANO CORRIGIDO

## BLOCO A — CONSOLIDAÇÃO (ANTES DE MAIS CÓDIGO)

1. Criar `ARCHITECTURE_COMPARISON.md`.
2. Atualizar `RENDER_DEPENDENCY_MAP.md` com contrato shader.
3. Reclassificar FASE 4–7 pelos estados DOCUMENTADO/CRIADO/INTEGRADO/COMPILADO/VALIDADO.
4. Marcar BoneUBO/4-influences como experimental.

## BLOCO B — SKELETON CORRETO

1. Auditar `New_ModelBMD::SendBonePalette`.
2. Auditar `Vertex_t::Node`.
3. Definir `SkeletonBuffer` compartilhado.
4. Implementar OpenGL `BonesTexture`.
5. Preparar interface backend-neutra.
6. Mapear `BoneIndex` por instância.
7. Integrar Models + Shadows.
8. Comparar CPU/GPU.

## BLOCO C — BUILD

Após cada integração física:

```
Global Release|x86
↓
corrigir erros
↓
abrir cliente
↓
teste visual
↓
commit
```

---

# VEREDITO FINAL

A modernização NÃO está errada, mas a auditoria encontrou uma mudança necessária de prioridade.

A melhor decisão agora é **consolidar**, não continuar criando camadas paralelas.

## Prioridade imediata

```
1. Contratos reais dos shaders
2. SkeletonBuffer compartilhado
3. Integração OpenGL real
4. Build e validação
5. Depois abstração Vulkan
```

O maior risco atual não é o código legado.

É acumular infraestrutura nova sem integração e sem testes.

---

# CHECKLIST PÓS-AUDITORIA

- [ ] ARCHITECTURE_COMPARISON.md
- [ ] Atualizar RENDER_DEPENDENCY_MAP.md
- [ ] Reclassificar PROGRESS.md
- [ ] Auditar SendBonePalette
- [ ] Auditar Vertex_t::Node
- [ ] Definir SkeletonBuffer
- [ ] Reconciliar BoneUBO experimental
- [ ] Mapear BoneIndex por instância
- [ ] Integrar Models + Shadows
- [ ] Adicionar fontes necessárias ao .vcxproj
- [ ] Compilar
- [ ] Testar OpenGL
- [ ] Preparar Vulkan somente após estabilidade
