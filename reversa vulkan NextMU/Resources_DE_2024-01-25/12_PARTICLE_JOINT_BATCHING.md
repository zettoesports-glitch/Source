# 12 — Particle e Joint batching

Status: **estrutura de batching CONFIRMADA; nomes de alguns campos RECONSTRUÍDOS via RTTI/EnTT.**

## Arquitetura comum

`NParticles::Render` e `NJoints::Render` possuem estruturas quase idênticas. Ambos:

1. consultam component storages EnTT;
2. atribuem metadata de render por entidade;
3. agrupam entidades por chave/grupo;
4. produzem comandos compactos de batch;
5. chamam um renderer por grupo;
6. fazem duas resource transitions quando o buffer foi alterado.

## Endereços

```text
NParticles::Render  0x140074170 - 0x1400749BA
NJoints::Render     0x1400AA1B0 - 0x1400AA9FA
```

## Componentes EnTT confirmados

Particles:

```text
TParticle::Entity::Info
TParticle::Entity::LifeTime
TParticle::Entity::RenderGroup
TParticle::Entity::RenderIndex
TParticle::Entity::RenderCount
```

Joints:

```text
TJoint::Entity::Info
TJoint::Entity::LifeTime
TJoint::Entity::RenderGroup
TJoint::Entity::RenderIndex
TJoint::Entity::RenderCount
```

## Hashes FNV1a/EnTT confirmados

```text
Particle RenderGroup 0x3E9EE040
Particle RenderIndex 0xE481FE3F
Particle RenderCount 0xF81A7C70
Particle Info        0x3DFB8509
Particle LifeTime    0xC2475E0A

Joint RenderGroup    0xD1276CDA
Joint RenderIndex    0xF10393F9
Joint RenderCount    0xC3C15222
Joint Info           0x1A5EAA5B
Joint LifeTime       0x0B855384
```

## Formato de batch observado

A fila cresce em `0x0C` bytes por entrada:

```cpp
struct RenderBatchCommand // clean-room name
{
    uint32_t Group;
    uint32_t Start;
    uint32_t Count;
}; // 12 bytes
```

Quando `Group` muda, o cliente resolve um renderer:

```text
Particles: 0x1400A3220
Joints:    0x1400B5540
```

Depois chama virtual `+0x28` uma vez por batch válido.

## Limites recuperados

```text
Particles: 0xC350  = 50.000
Joints:    0x3D090 = 250.000
```

Quando o acréscimo ultrapassa o limite, metadata relacionada recebe `0xFFFFFFFF`, evitando ultrapassar o buffer reservado.

## Resource transitions

Particles:

```text
0x1400748D9 -> GetCurrentDeviceContext
call [vtable + 0x1A0] = TransitionResourceStates
count = 2
```

Joints:

```text
0x1400AA919 -> GetCurrentDeviceContext
call [vtable + 0x1A0] = TransitionResourceStates
count = 2
```

Ocorre somente quando dirty flag indica mudança nos buffers.

## O que vale portar

- ECS/SoA para metadata quente;
- batch command compacto;
- renderer lookup somente ao mudar grupo;
- capacidade explícita do buffer;
- dirty flag para transitions;
- transition agrupada após atualizar o conjunto;
- uma chamada de draw por batch, não por entidade.

## Clean-room target

```text
Update entities CPU
      ↓
Build RenderBatchCommand[]
      ↓
Upload/Map buffers uma vez
      ↓
Transition resources (se dirty)
      ↓
for each batch:
    Bind group/material quando muda
    Draw/DrawIndexed(batch range)
```

É uma das otimizações mais úteis encontradas para cenas de skills/eventos com muitas entidades.
