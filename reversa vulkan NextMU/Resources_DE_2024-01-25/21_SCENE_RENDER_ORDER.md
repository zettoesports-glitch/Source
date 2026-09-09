# 21 — Ordem real de preparação e renderização da cena

Status: **CONFIRMADO** pelo fluxo x64 do `NextMU.exe`.

## PreRender / preparação

Na fase de preparação, o cliente processa os managers antes do replay gráfico:

```text
Terrain preparation
Characters::PreRender   0x1400678E0
Objects::PreRender      0x1400A5A20
Particles preparation
Joints preparation
```

Objetos membros observados na classe/cena:

```text
+0x538 Terrain
+0x558 Objects
+0x560 Characters
+0x568 Particles
+0x570 Joints
```

## Ordem de Render confirmada

A mesma associação de membros permite fechar a sequência:

```text
1. Terrain     -> 0x1400516C0
2. Objects     -> 0x1400A68B0
3. Characters  -> 0x140068350
4. Particles   -> 0x140074170
5. Joints      -> 0x1400AA1B0
6. command buffer replay -> 0x14009A0F0
```

Após o replay, o frame segue para as demais etapas/UI e finalmente `Present()` conforme documentado no lifecycle.

## Por que isso é importante

A ordem não deve ser alterada livremente no nosso port. Ela influencia:
- depth test;
- objetos opacos;
- personagens;
- transparência;
- particles;
- joint ribbons/effects;
- composição final.

Uma otimização de sorting deve respeitar barreiras entre classes/passes, e não ordenar a cena inteira por textura/PSO.

## Observação importante

`Objects::Render` e `Characters::Render` convergem para o mesmo renderer de Model/BMD (documentado separadamente). Isso significa que a diferença de gameplay/entidade acontece antes da etapa comum de desenho.

## Regra recomendada para nosso frontend

```text
ScenePass::Terrain
ScenePass::Objects
ScenePass::Characters
ScenePass::Particles
ScenePass::Joints
ScenePass::UI
```

Dentro de cada pass, otimizações e batching podem ocorrer desde que não quebrem dependências de blend/depth/ordem.