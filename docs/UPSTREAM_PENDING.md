# Framework changes pending upstream review

Updated: 2026-10-02. Development branch: `vr-dev`; push remote: `fork`.
Reference upstream base: `3505f2a0` (`master`). The Medal of Honor project pins
this branch through its framework submodule. This file inventories pending work;
it does not assert upstream acceptance or stereo completion.

## Paired stereo enhancement

Implementation commit: `0a955971`. Live evidence is in the game repository's
`vr/proof/stereo-pairs/`, recorded with the game plugin changes accompanying it.

The new paired API shares the existing frozen-time transaction and restore
internals, but has explicit eye IDs and no interpolation or phase-plan gate.
Two staging textures become visible together only after both eyes complete.
Failure discards staging and retains the preceding complete pair; host cost
shedding skips whole pairs. Ordinary OpenGL 15-bit GPU-authoritative rendering
is supported initially; native-wide, window tiles, depth24 and dual raster are
refused. Side-by-side presentation is explicitly enabled by the plugin.

A scoped camera-space GTE view offset is added before RTPS/RTPT perspective
division without rewriting guest TR registers. The transaction restores this
host ambient on normal return and watchdog rollback. Its default is zero.
TCP `stereo_stats` and `stereo_dump` expose eye checkpoints, actual sampled
offsets, pair outcomes, retained-pair failure records, textures and manifests.
The temporal API retains its existing gates and phase contract.

Files: `runtime/include/{mod_plugins,render_pass,gpu_gl_renderer,gte_view}.h`,
`runtime/src/{render_pass,gpu_gl_renderer,debug_server}.c`, `runtime/src/gte.cpp`,
render-pass and GTE tests, `docs/STEREO_RENDERING.md`, generated command index.

Validation: sandbox, nested watchdog and source guard tests passed.
The right-eye nested watchdog test retains the previous pair, restores CPU/RAM/
host nesting/view ambient and permits the next pair. GTE tests measure 12px
versus 3px at depths 800 and 3200, match explicit pre-divide translation, and
verify unchanged TR and repeatability. OpenGL game Debug and Release builds pass.
Live slot-3 captures with two enemies give identical decoded eyes at zero
separation. At +/-24, inspected wall/near-ground ROIs correspond at 5/13px,
with equal entry cycles/hashes and zero restore mismatches/leaks/dropped stores.
The first 96 post-load fingerprint columns/cycles match the no-redraw control
for zero, offset, held-watchdog and recovery runs. A right-eye watchdog inside
real guest dispatch retains published pair 1 with staging mask zero; later
pairs recover and the failed-eye/retained-pair record remains latched. Composed
SBS readback succeeds with interpolation disabled and no temporal plans.

Debug pair samples around 35ms exceed the conservative two-VBlank budget and
shed whole pairs. This is bounded correctness evidence, not a headset throughput
claim. OpenXR/head poses, scale/IPD and HUD comfort remain game/backend work.
Native-wide, alternate display modes and other games have not been validated.

Release with TCP tools enabled and VERIFY off was measured separately at actual
internal scale 5 (2560x1200 eye textures): warm receipt 1,719 pairs, no shedding
or failures, last pair 12.013ms / EMA 11.584ms. Its 96-frame fingerprints match
a separate Release OFF control at identical settings. Cross-build fingerprints
differ and are retained, rather than treated as an eye-redraw comparison.
No state-hash verification or headset frame-time-tail claim is made for this
cost sample. The original Release config without TCP tools yielded no measurement.

Follow-up also corrects checkpoint-refusal status sampling to use the selected
stereo/temporal gate; stereo failure diagnostics must not report an interpolation
requirement. Sandbox/abort tests pass after that diagnostic correction.

## Render-pass refusal diagnostics

Commits: `257a88b0` (implementation), `ee7a4afc` (live verification notes).

Previously a rejected `psx_mod_render_pass()` returned zero without identifying
which pre-callback gate failed. `render_pass_stats` now exposes `pass_attempts`,
`argument_refused`, `status_refused`, `begin_refused`, `checkpoint_refused` and a
latched `last_failure`. Existing `refused` still counts empty temporal plans.

The failure record includes attempt/plan/cycle identifiers, requested rect and
phase, rejection status, actual GL capture dimensions, scales, generation state,
resource stage, framebuffer status and allocation-local GL errors. Successful
calls preserve the last failure; session reset clears it. Checkpoint allocation
failures are distinguished from GL begin failures. Instrumentation is exposed
through the TCP debug server; the existing FBO-failure printf was removed.

Files: `runtime/include/{gpu_gl_renderer,render_pass}.h`,
`runtime/src/{gpu_gl_renderer,render_pass,debug_server}.c`, render-pass tests,
`docs/RENDER_PASSES.md`, generated `docs/TCP_COMMANDS.md`, and timing session notes.
The generated command index also picks up the previously unlisted `disasm` handler.

Validation:

- Render-pass sandbox and abort tests passed; guard checks passed.
- TCP command index regeneration check passed; Medal of Honor OpenGL Debug build passed.
- Live refusal record measured requested 512x240 versus capture history 256x240,
  with both scales 1. The guard correctly refused; no allocator failure was measured.
- A game-only hook at its gameplay render-wait entry produced 727 no-op passes
  and verification checks with zero mismatches, aborts or pass-call refusals.
  Two promoted baseline/pass pairs had zero differing decoded RGB pixels.
- Game commit `93be7f2` adds bounded live fingerprint comparison: two control
  loads, two no-op loads and a synthetic-watchdog load match every measured
  fingerprint column and cycle count for 96 post-load frames.
- Game commit `4c79f77` reconstructs the real slot-1 draw slice without wait/flip.
  Clearing the rect then rebuilding the scene gives two pixel-identical replay
  pairs. Clear-only/level-only controls isolate coverage. Full draws and an abort
  inside real level dispatch match the same 96-frame control timeline; the abort
  run records one watchdog, 388 subsequent successes and zero restore mismatches.

Evidence lives in the game repo at `vr/proof/pass-diagnostics/`; game commit
`5e938a3` contains the initial probe and receipts; `vr/proof/replay-scope/` and
`vr/proof/scene-replay/` contain subsequent timeline and draw evidence. These
results establish no-op transactions and a bounded room/weapon/HUD redraw in
slot 1, not animated-object coverage, every scene mode or stereo. Allocation
diagnostics record errors but do not change the existing `glTexImage2D` success
policy. Paired-eye capture/presentation remain in the game execution plan.

Suggested PR scope: refusal diagnostics, tests and associated documentation.
Keep game-specific hooks and stereo presentation out of this PR.

## Guest disassembly over TCP

Commits: `85c47e10`, `d58db909`, `c2ed6e55`.

Adds `disasm addr=0x... count=N`, backed by a C/C++ shim using the existing
instruction decoder. Follow-ups correct SPECIAL/R-type formatting and name GTE
commands and COP2 registers. Includes runtime build integration and public header.
The live draw-loop investigation uses this command; automated decoder-format
coverage should be assessed before opening its separate PR.

## Optional GTE projection-distance scale

Commits: `82695b75`, `5633e868`, `bbd01ccf`, `39d478db`.

Adds default-identity projection-distance scaling to GTE RTPS/RTPT, configured
through `[video] fov_scale` or `PSX_GTE_FOV_SCALE` (environment takes precedence).
A value greater than 1 divides effective H and widens perspective; it is not an
exact multiplier of the angle in degrees. GTE trace records report effective H.
Guest control-register storage is unchanged by this host projection enhancement.

Review before PR: numerical edge cases, environment validation versus config
validation, identity-path regression tests and documentation of trace semantics.
This enhancement alone does not provide a stereo viewpoint.

## Netplay-disabled link fix

Commit: `ce63101f`.

Adds missing `psx_lobby_online_count` and `psx_lobby_online_get` stubs for
`PSX_NETPLAY=OFF`. Suitable for a small independent PR; validate both build modes.

## Reverted experiment

`2c919f08` added a GTE vertex-capture seam; `cec02977` reverted it. There is no
remaining vertex-capture implementation in the branch diff. Exclude both from
an upstream implementation PR.

## Delivery convention

Document every further framework change here with its purpose, affected API,
validation and outstanding limitations. Push each commit to `fork/vr-dev` before
updating and pushing the game submodule pin. Do not mix pending enhancements into
faithful defaults. The simultaneous paired-image contract now shares transaction internals;
further headset work must retain that contract and the faithful defaults.


## Scoped rigid views and experimental OpenXR (2026-10-02)

Pending PR: PSXModRenderView adds a Q12 rigid camera transform and asymmetric
Q16 projection, with optional authored H/reference focal ratio. Shared render
transactions restore the full host pose on return/decline/watchdog; guest TR
and CPU layout are unchanged. Identity and the existing offset path retain
canonical GTE results. Metric pose conversion is independently tested.

New opt-in PSX_OPENXR Win32/GL backend uses the pinned official SDK/static
loader, explicit frame lifecycle, located eye poses/FOV, LOCAL recenter,
private acquired swapchain images, matching projection submission and teardown.
A fresh complete pair is required; rejected redraws submit zero layers.
OpenXR launches explicitly request GL 4.6; ordinary launches keep 3.3.
The MinGW-only vendor -Wundef warning is left nonfatal for older Windows
partition headers; runtime warning policy is not changed. No SDK vendor edits.

TCP openxr_stats/openxr_views/openxr_control expose the producer values and
failures. GTE projection inspection adds render tagging, pagination and a 4096
record limit for full-frame domain inspection. New docs/OPENXR_RENDERING.md
records contract, measured Quest 3/VDXR startup/version refusal, Y-copy
correction, actual IPD, and remaining cadence/culling/controller/HUD limitations.

Validation: GTE canonical/register oracle and rigid/projection/focal tests,
metric-pose tests, ordinary and watchdog full-pose restore tests, render guards,
TCP index check, Debug/Release Medal of Honor builds, slot-3 paired controls and
96-frame equal timelines. Live VDXR Release submitted complete native
projection pairs; user confirmed upright output and expected head tracking.
Game retains detailed receipts/results in docs/reverse; bulk captures ignored.
World scale and wrist HUD are not declared physically calibrated/complete.

## OpenXR locomotion actions and trusted offline controller sources (2026-10-02)

Adds left/right Touch thumbstick subactions, pre-start action-set attachment,
normal-input-boundary synchronization and neutral release on unavailable,
unfocused, inactive or failed action state. The public PSXModOpenXRInput API is
independent of eye replay. TCP openxr_input observes actual samples;
openxr_input_override provides explicitly synthetic desktop controls only in
debug-tool builds. The generated TCP index now has 330 commands.

PSXModControllerState/psx_mod_set_controller_source let a trusted game plugin
own offline axes/type and contribute buttons while retaining local menu buttons,
coherent SIO mode requests, post-load suppression and selfcheck input recording.
Existing TCP overrides win. Netplay/resim/eye replay do not poll sources. Decline,
invalid data and detachment deliver neutral; session activation resets sources.
No faithful default changes. Files: mod_plugins.h, mod_controller_source.h/.c,
main.cpp, psx_openxr.h/.c, gpu_gl_renderer.c, debug_server.c and CMake integration.
Game-specific axis interpretation, deadzone and gain stay outside the framework.

Validation: controller source and compiled-out XR-input C tests; game mapping C
test; existing controller lifecycle/render guards; debug-less compiled-out syntax;
TCP index check; OpenXR-enabled Debug/Release builds. Actual slot-3 native axis
controls and 11 synthetic action controls confirm movement/turn/release. Neutral
stereo OFF/ON samples match all 96 fingerprint columns/cycles. Debug's 86 verified
eye restores have zero mismatches/leaks/dropped stores. Live Quest/VDXR records
257 adjacent action/pad snapshots, synthetic=0, both horizontal directions and
left vertical directions; 3,131 submissions with zero XR failures or stereo
shedding in its bounded sample. Adjacent queries are not atomic delivery proof.

Outstanding: real focus-loss/reconnection controls, other game input schemes,
controller buttons/poses/aiming, final sensitivity tuning, and independent
headset cadence. A restricted launch reported xrGetSystem=-35; a user-session
launch subsequently succeeded. Do not infer a controller disconnection or a
specific IPC cause from that startup code. Game receipts record launch provenance.

The initial game-side centered-byte mapping produced asymmetric turns and weak
diagonals. Native converter tracing established its nonlinear asymmetric curve;
that fix stays game-owned (live calibrated inverse and radial deadzone), without
framework changes. Actual heading SW increments match +/-2,457,600 at full Quest
turn and +/-1,290,240 at the half-range control; user confirmed corrected movement
fully consistent. Physical angular speed and real focus-loss/reconnection remain
unmeasured. A later visibility complaint occurred despite valid submitted pairs;
restart restored the test, but its cause was not isolated. Do not infer visibility
from submission counters alone.

## OpenXR Touch combat action samples (2026-10-02)

Extend the opt-in gameplay action set from sticks to trigger/squeeze floats and
primary/secondary/Menu/stick-click booleans, with 13 Oculus Touch bindings.
Activity is per action, independently of stick activity. No game semantics or
PSX button assignments enter the XR backend. All unavailable/focus-loss/failure
paths release values; synthetic overrides validate ranges/masks before mutation.

Files: runtime/include/mod_plugins.h, runtime/src/psx_openxr.c,
runtime/src/debug_server.c, runtime/tests/test_openxr_input.c,
docs/OPENXR_RENDERING.md and docs/internal/FAITHFUL_TIMING_PLAN.md. The public
input struct has appended fields and still requires current sizeof: rebuild
consumers. Existing TCP commands expose the new fields and synthetic controls;
the command count stays 330. No new printf/log diagnostic path or faithful default
change. The game mapping and native binding guard remain outside this repo.

Validation: compiled-out XR-input test with independent stick/button activity,
invalid values and neutral release; game mapping test including combined movement
and fire; debug-less syntax; controller lifecycle/render guards; TCP index;
OpenXR-enabled Debug/Release builds. Live desktop synthetic controls establish
weapon selection, ammo decrement/reload transfer and crouch through actual native
writers, plus button release and pause/resume. Real Quest combat actions and
hardware focus-loss/reconnect are pending; prior live stick measurements do not
validate the added actions. Raw game evidence stays ignored, compact receipt in
game docs/reverse/VR_COMBAT_RECEIPT.json. First-generation headset cadence and
native body aim are unchanged.
