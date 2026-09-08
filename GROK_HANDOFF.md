# GROK HANDOFF — MU Online ModernBMD modernization

## Read this first

You are working on repository `zettoesports-glitch/Source`.

**Work only on branch `modernization`. Never modify `main`.** `main` is the immutable legacy baseline.

Before any edit:

```bat
cd C:\MUVULKAN\Source
git checkout modernization
git pull --ff-only
git status --short
git rev-parse HEAD
```

Do not force-push, reset `main`, mass-format files, or rewrite unrelated code. Keep commits small and descriptive. If ChatGPT and Grok are both editing the repository, serialize writes: pull immediately before editing and again before the next agent starts. Do not have both agents replace the same file concurrently.

## Project goal

Modernize the MU Online 5.2/SSeMU renderer gradually from legacy OpenGL/fixed-function behavior toward modern OpenGL/RHI/GPU skinning while preserving gameplay, protocol and BMD compatibility. The architecture should remain future-ready for Vulkan, but **do not start Vulkan now**.

Do not mass-convert `glBegin`/legacy rendering. Migrate subsystem by subsystem and keep the legacy path as a reliable fallback.

## Hard rules / invariants

- `MAX_BONES = 200`.
- BMD `Vertex_t::Node` currently provides one bone influence per vertex; modern representation uses weight 1 until real weights exist.
- Modern skeleton atlas default encoding is `Matrix4x4`; QPS remains optional.
- Generated shader contract: Vulkan-main HLSL -> generated OpenGL GLSL.
- Do not globally enable `jdk_shader_local330` until validation is complete.
- Keep the material-coherence guard. Do not mix a modern base pass with legacy overlays on the same object.
- NPC/monster stay forced legacy for now.
- Local Hero is generally modern-eligible, except during the explicit remote-rollout isolation diagnostic below.
- Remote players/BotBuffer safe default in code remains legacy (`ForceLegacyRemotePlayers=1` when the config key is absent).
- Never globally quarantine `Kind == 0`. It is also used by weapons, skills and helper objects and previously caused world/object regressions.
- Do not add broad pet/world-object quarantines to solve one asset.

## Important existing fixes

### Immutable transform snapshot

Modern commands no longer depend on mutable `BMD::BodyScale` / `BodyOrigin` during a later flush. `RenderMeshVAO` captures the transform together with the bone palette. Main commit in that line of work: `9e7b1d92537bf66bfcd4d01c8bb10b7abb8afedf`.

### Equipment / external player skeleton palette

`RenderPartObject()` can render an equipment BMD using the owning player's `o->BoneTransform`. Therefore the equipment model's own `NumBones` is not a valid upper bound for the matrices its vertices may reference.

Commit `8fa21fe346cad044aa287867969c23eaed1dfca5` expanded captured palettes to the highest actually referenced `Vertex_t::Node + 1`, clamped by `MAX_BONES`.

Commit `e13faf2045576711a3fdfb14b2dcc74d678757d6` rejects modern meshes whose bone references exceed the supported palette.

Validation invariant: if a mesh log reports `maxBone=N`, its captured modern palette must have at least `N+1` bones.

### merchant_f culling workaround

`Data2\Npc\merchant_f.smd` contains thin one-sided clothing geometry. A scoped legacy double-sided override exists specifically for that asset. Do not turn off culling globally.

### Animated world tree

`Data2\Object52\sos3bi01.smd` (28 bones) produced exploded geometry on the ModernBMD path. It is quarantined specifically by asset name with reason:

```text
reason=world-asset-sos3bi01
```

Do **not** replace this with `Kind == 0` filtering.

## Deterministic build/test workflow

Always build with:

```bat
BuildAndRunModernClient.bat
```

That script now:

1. checks critical renderer sources for local modifications;
2. performs `/t:Rebuild` for `Global Release|x86`;
3. copies the exact build output to `Cliente\Main.exe`;
4. checks SHA256 source/deployed equality;
5. clears and stamps `Cliente\Data\ModernBMD.log` with Git HEAD + SHA256;
6. launches with `Cliente` as working directory.

A valid test log must start with something like:

```text
[ModernBMD] build-head=<git sha>; build-mode=Rebuild; sha256=<hash>
```

If the logged HEAD is not the branch HEAD, stop. Do not diagnose visuals from that run.

## Current isolated BK diagnostic

Current repository config intentionally contains:

```ini
[ModernRenderer]
ExperimentalBMD=1
MinBones=20
MinActions=2
TargetName=
ForceLegacyRemotePlayers=0
RemotePlayerClass=1
RemotePlayerSingleObject=1
RemoteRolloutIsolation=1
Diagnostics=1
```

Class map used by the guard:

- `0` Wizard/SM
- `1` Knight/BK
- `2` Elf
- `3` Magic Gladiator
- `4` Dark Lord

`RemoteRolloutIsolation=1` is a **diagnostic mode**, not the desired final production setting.

The renderer now maintains the active `rRenderLayOut` OBJECT scope. While isolation is enabled, `CGMShaderBMD::Render()` may call the ModernBMD runtime only when that active scope is the single remote player selected by the rollout guard. Other world objects and the local Hero continue through the existing legacy/u_Bones shader fallback instead of being globally disabled.

This was added because a clean validation run correctly selected one BK but the first modern draw in the frame was still an unrelated animated world model (`Data2\Object79\sos3bi03.smd`). The diagnostic must isolate the BK itself, not merely allow it while unrelated modern objects continue drawing.

Expected log for the selected BK:

```text
object-instance rollout: remote-player/bot allowed to modern candidate ... class=1 className=knight classFilter=1 singleObject=1 selected=1
```

Other BKs should show:

```text
reason=remote-single-object-filter
```

Other classes should show:

```text
reason=remote-class-filter
```

The quarantined tree should show:

```text
reason=world-asset-sos3bi01
```

During `RemoteRolloutIsolation=1`, unrelated world assets should not produce `atlas modern BMD selected:` as an actual modern draw. The selected BK/body-part BMD should become the first actual modern selection if it is eligible and the material-coherence guard allows the batch.

## What to inspect if the BK is still deformed

Do not add another broad quarantine. Work from the selected BK only.

1. Confirm the run is from the stamped current HEAD and a full Rebuild.
2. Confirm `selected=1` for exactly one Knight/BK.
3. For every modern BK/body-part mesh, verify `paletteBoneCount > maxBone` (equivalently at least `maxBone + 1`).
4. If bone bounds are valid but geometry is still wrong, the next leading suspect is **command/palette ownership/association**, not atlas arithmetic.
5. Inspect the standard equipment/body path in `ZzzCharacter.cpp` and `RenderPartObject()` in `ZzzObject.cpp`. Equipment/body models use the player's external `o->BoneTransform`.
6. Inspect linked render paths such as weapons, wings/back items and `RenderLinkObject`. A global "last AddBoneTransform" state can associate a mesh command with the wrong palette if a linked object changes the current capture between transform and command creation.
7. Prefer making the owning `OBJECT*` / palette association explicit per immutable render command rather than adding model-name exceptions.
8. Preserve the material-coherence guard while doing this.

Potential architecture if association is confirmed wrong: snapshot explicit render-owner identity and palette identity into each `RenderMeshVAO`, so every mesh command carries the exact `OBJECT*`, `BMD*`, transform and palette that produced it. Do not read mutable global/BMD state later during flush.

## What NOT to do

- Do not modify `main`.
- Do not start Vulkan.
- Do not disable the material-coherence guard.
- Do not make all `Kind == 0` objects legacy.
- Do not globally disable culling.
- Do not remove `MAX_BONES` validation.
- Do not assume `model->NumBones` bounds external player skeletons.
- Do not turn all pets legacy to fix one effect.
- Do not change network/gameplay/protocol code for a renderer visual bug.
- Do not interpret an old/incremental binary as current source behavior.

## Coordination protocol for Grok

For every change:

```bat
git pull --ff-only
git status --short
```

Fetch/read the exact `modernization` version of a file before editing. After editing, commit only the intended files with a descriptive message. Then report:

- commit SHA;
- files changed;
- exact hypothesis being tested;
- expected log signature;
- rollback key/config if applicable.

Before handing control back to ChatGPT or another agent, make sure the branch is pushed and provide the new HEAD. Never leave uncommitted renderer changes on the user's machine for another agent to unknowingly build.

## Main files in the current investigation

- `Main/source/New_ModelBMD.h`
- `Main/source/New_ModelBMD.cpp`
- `Main/source/New_RenderBMD.cpp`
- `Main/source/Render/Model/BMDModernRuntime.cpp`
- `Main/source/Render/Model/BMDModernObjectGuard.cpp`
- `Main/source/Render/Model/BMDModernMesh.cpp`
- `Main/source/Render/Model/BMDModernSkeletonAtlas.cpp`
- `Main/source/Render/Model/BMDModernSkeletonPose.cpp`
- `Main/source/Render/Model/SkeletonBuffer.cpp`
- `Main/source/Render/Model/SkeletonEncoder.cpp`
- `Main/source/Render/OpenGL/OpenGLBMDModernVAO.cpp`
- `Main/source/Render/OpenGL/OpenGLBMDModernInstanceBuffer.cpp`
- `Main/source/Render/OpenGL/OpenGLSkeletonTexture.cpp`
- `Main/source/Render/OpenGL/OpenGLBMDModernBindings.cpp`
- `Main/source/ZzzCharacter.cpp`
- `Main/source/ZzzObject.cpp`
- `Cliente/Data/Custom/config.ini`
- `BuildAndRunModernClient.bat`

## Immediate next test

Pull the latest `modernization`, run `BuildAndRunModernClient.bat`, inspect the selected BK visually, and save the fresh `Cliente\Data\ModernBMD.log`.

If the BK is correct, repeat the same isolated test with:

```ini
RemotePlayerClass=2
```

for one Elf. If the BK is still wrong, follow the ownership/palette investigation above before expanding rollout.
