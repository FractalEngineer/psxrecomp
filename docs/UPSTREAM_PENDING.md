# Framework changes pending upstream review

Updated: 2026-10-03. Development branch: `vr-dev`; push remote: `fork`.
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

Follow-up hardware validation: the Quest 3/VDXR user confirmed all mapped combat
controls. A separate 246-row non-synthetic sample caught right squeeze up to 1.0
and adjacent native R2 delivery, followed by unfocused neutral action/pad samples.
Other presses were outside this capture; distinguish user acceptance from trace
proof. Trailing status queries failed after the bounded process closed. Startup
receipts have live projection submissions and zero XR failures at that snapshot;
these are not whole-test throughput or visibility proof. The user reports the
pause menu too close and intends to replace native grip aim with weapon aiming.
Controlled focus-loss/reconnection, controller poses/shot aiming and independent
cadence remain open. No framework runtime changes for this follow-up.

## Controller grip/aim snapshots and producer-PC trace filtering

2026-10-02 follow-up on vr-dev. `PSXModOpenXRHands` is a separate opt-in
read-only snapshot API, leaving PSXModOpenXRInput unchanged. Four grip/aim
action spaces are located at the eye frame's predicted time in LOCAL, with
the rendered recenter origin, independent activity/validity/tracking flags and
host age. Querying does not sync actions or locate poses. Clearing on invalid
frame/focus/recenter/shutdown prevents reuse; game policy must check freshness
and validity before affecting gameplay. The helper pose-to-transform is the
inverse of the existing recentered PSX view. No game weapon mapping, polling
source or faithful default is added. Debug synthetic poses remain labelled.

TCP `openxr_hands` and `openxr_hands_override` expose these contracts. The
existing `wtrace_dump` also gains post-hoc full recorded PC bounds (inclusive
lo/exclusive hi), applied before output count. DMA uses its already recorded
initiator PC. Recording, guest execution and fingerprints are unchanged.
This addresses truncated high-traffic replies when isolating shot/damage
producers; an empty filtered reply alone is not absence-of-execution proof.

Files: runtime/include/{mod_plugins,psx_openxr,vr_pose_math}.h,
runtime/src/{psx_openxr,vr_pose_math,gpu_gl_renderer,debug_server}.c,
input/math tests, OPENXR_RENDERING.md and TCP_COMMANDS.md (332 commands).

Validation: compiled-out input/lifecycle and pose-math tests passed, as did
debug-less syntax with the existing unused-parameter exception. SDK-enabled
Release builds and live synthetic snapshot checks passed (stable sequence,
invalid quaternion non-mutation, partial flags, focus and clear). Live PC
filter matched Python filtering of a complete recorded slice, including empty
range and newest/count checks. A native rifle control in MoH slot 5 measured
enemy health 6 -> 3.5 at SW 8004ACC0, with idle/turned-away controls unchanged.
These are desktop producer/diagnostic controls, not actual Quest pose alignment
or aiming acceptance. Compact receipts reside in the game docs/reverse.

2026-10-03 real-device follow-up: Quest 3/VirtualDesktopXR provided 242 adjacent
TCP hand/action/pad samples over 30 seconds. All right grip/aim snapshots were
non-synthetic, focused, origin-valid, active and orientation/position-valid,
with ages 0..29ms. Right trigger reached 1.0. Final read-only counters recorded
6,805 XR submissions and zero failures; stereo recorded zero failed pairs or
watchdogs. This establishes real action-space delivery, not physical weapon
alignment, shot direction, headset-rate cadence or restore verification (verify
was off). The native enemy-health slice began after health was already zero,
so it supplies no damage/alignment control. Visual/user acceptance is pending.
The game was closed. No framework runtime changes; tested binary uses 35b209d4.
Compact game evidence: docs/reverse/VR_WEAPON_QUEST_RECEIPT.json.

## Render-pass rollback of nested mod callback context

2026-10-03, vr-dev. A watchdog longjmp inside nested function filters skipped
the mod runtime's callback depth/owner exits. Guest restore and later stereo
pairs succeeded, but callback depth stayed elevated and deferred savestate
loads could never reach their safe boundary. RenderPassNesting now checkpoints
and restores the exact interrupted depth and plugin owner, including a nonzero
outer callback. These host pointers are transaction-local, never serialized
in guest saves. Verify mode also detects normal-return imbalance.

Files: runtime/include/{mod_runtime,render_pass}.h,
runtime/src/{mod_runtime.cpp,render_pass.c,debug_server.c}, abort/sandbox tests.
TCP render_pass_stats now latches last_abort_detail across successes, until
session reset; no new command or printf instrumentation. Default guest timing,
input sources and native rendering are unchanged.

Validation: strict GCC abort/sandbox tests pass, including five nested mod
callbacks, owner changes, a nonzero outer context, right-eye failure retaining
the previous pair, later recovery and a mod-only normal-return imbalance.
Source guards and the 332-command TCP index pass. OpenXR-enabled Release built.
Live MoH slot-5 weapon-wrapper right-eye watchdog reported `mod entries +2`,
one nesting repair and 52 restore checks with zero mismatches. A subsequent
slot-5 load advanced generation 1 -> 2 with pending=0 and last_ok=1; later
pairs recovered. Raw evidence is ignored in game analysis/vr-proof/
weapon-wrapper-fixed; compact game receipt will retain these results.


## Frame-local OpenXR menu quad (2026-10-03)

Add psx_mod_openxr_quad(distance,width,height), a generic application-requested
UI submission mode. It uses VIEW space and the fresh stereo pair's left image
for both eyes, retaining the atomic pair gate and empty-frame failure behavior.
Requests are finite/range checked, refused inside replay and without a tracked
open frame, and reset each frame. Zero distance restores projection. Normal
projection rendering, input, guest timing and default faithful behavior stay
unchanged. VIEW space is lazy and destroyed with the session.

Files: runtime/include/{mod_plugins,psx_openxr}.h,
runtime/src/{psx_openxr,gpu_gl_renderer,debug_server}.c,
runtime/tests/test_openxr_input.c, OPENXR_RENDERING.md and timing log.
Existing TCP openxr_stats exposes submitted_layer, quad_submitted and latched
quad dimensions; no printf instrumentation or new command (332 total).

Validation: strict compiled-out input tests, debug-less syntax (existing
unused-parameter exception), render-pass guards and TCP index pass. Both SDK
builds pass. Desktop game pause produces identical decoded eye images with
408 restore checks/zero mismatch. A separate injected weapon-wrapper watchdog
restored the original 280 faces, repaired nesting, passed 932 restore checks,
and allowed a generation-2 slot reload. These are game transaction controls,
not a live failed-quad test. Quest 3/VDXR submitted 89 quads at distance 2m,
width 2m, height 1.5m and resumed projection at the first inspection; user
confirmed rifle appearance and menu comfort. Native shot alignment remains
unvalidated. Full game receipts remain under game docs/reverse.

Startup correction: sandboxed runs returned system -35 with zero submissions,
whereas the otherwise identical external launch succeeded. Do not report that
as evidence the user's active headset was unavailable. No general XR retry or
runtime selection change is included in this patch.


## Native OpenXR boot/menu/video surface (2026-10-03 checkpoint)

Generic `psx_mod_openxr_native_surface(distance,width,units)` selects a persistent
native-screen VIEW-space quad. Actual fresh VRAM/wide/CPU/blank presentation is
copied before host OSD, without replay or a retained stereo pair. Hold-last and
interpolated/stereo sources cannot masquerade as a fresh native frame. The
application disables the surface before gameplay begins its ordinary fresh-pair
projection submission. Distance zero disables it; finite/range checks, replay
and open-frame gates apply. Native UI pumps XR even for unchanged screen pixels.
Default rendering and guest timing are unchanged.

Files: runtime/include/{mod_plugins,psx_openxr,gpu_gl_renderer}.h and
runtime/src/{gpu_gl_renderer,psx_openxr,debug_server}.c. Existing TCP openxr_stats
adds submitted_source (1 pair, 2 native), native_submitted and
submitted_native_frame. Native sources have pair/cycle zero. video_info adds
actual gl_swap_interval (-2 unavailable). No new command or printf inspection.
The GL regression runner links real compiled-out XR cleanup; off-XR stats tests
check neutral native metadata. Validation used runtime mirrors in the game's
6134f8b8 submodule before this checkpoint. User authorized committing and pushing
the framework first, then updating the game pin.

Validation: SDK Debug/Release builds, strict compiled-out XR/menu input tests,
render guards and 332-command index pass. Quest/VDXR boot snapshot: 554 native
quads, zero empty/failure, save generation 0/last slot -1. A later natural
handoff reaches fresh-pair projection (4029 total XR submissions at inspection).
User accepted all menus and controls. First native attempt incorrectly supplied
1856x1392 drawable dimensions to PSX view math and submitted zero layers. Fixed
by keeping unused PSX matrices at 512x240 while copying full drawable content.
This is a recorded correction, not evidence of headset unavailability.

The game launcher alone disables desktop VSync for XR, retaining the guest
real-time deadline cap. Native MDEC interval controls: actual interval 1 yields
49.547 guest Hz / 11.776 decode Hz; actual interval 0 yields 59.195 / 13.686.
Turbo is off in both. User confirms sound/framerate fixed across boot/menu/
briefing. Adjacent TCP timing is not compositor or audio-underrun telemetry.
An optional native-texel bicubic trial passed source-owned GL controls at 1x/4x
and measured 58.462 guest Hz, but user preferred the earlier presentation;
filter implementation was removed and original presentation restored.

## OpenBIOS generation stamp for the native alpha (2026-10-03)

The FOV configuration parser changed an input covered by the BIOS emitter
fingerprint. Rebuilt both emitters from the current source, regenerated OpenBIOS
from the pinned redistributable image, and compared against the committed output:
every generated C/dispatch file is unchanged; only OpenBIOS.emitter.sha changes.
No BIOS instructions, runtime implementation or faithful defaults changed.

The game release links OpenBIOS only, matching the distributed image. The retail
SCPH1001 backend has not been regenerated: its input image is unavailable here,
and its older stamp remains stale. Do not claim both BIOS stamps pass or silently
refresh the retail stamp. The bundled overlay toolchain retains the profiles and
emitter for users supplying their own supported retail BIOS.

## Windows release executable selection (2026-10-03)

Shared tools/package_game_release.sh now prefers the literal .exe candidate
before the unsuffixed POSIX candidate, including OUTPUT_NAME marker fallback.
Git Bash reports -f true for an unsuffixed alias of a Windows executable; selecting
that alias stripped .exe from EXE_BASENAME and skipped DLL bundling/signing.

Actual alpha verification reproduced the alias and a stripped-PATH startup
failure 0xC0000135 with missing zlib1__.dll. Packaging with the corrected candidate
order identifies the PE and stages the imported DLL; the normal signing gate also
runs. This is a generic packaging fix, independent of VR runtime behavior.
