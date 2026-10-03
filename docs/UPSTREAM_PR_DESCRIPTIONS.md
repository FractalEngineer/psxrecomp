# Upstream PR descriptions

## PR 488: runtime: fix missing online-player lobby symbols in netplay-disabled builds

**Current behavior:** Netplay-disabled builds reference two lobby functions with no implementation.
**Desired behavior:** They link and report zero online players without the lobby client.

Fix the missing netplay-disabled lobby stubs so the launcher links without the lobby client.

Scope: game-agnostic runtime fix.

Validation: Windows GCC 13.2 Release; disabled-lobby compile/link/executable checks and enabled-lobby compilation passed.

Limit: a full enabled-netplay session was not run.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/fix/netplay-disabled-online-stubs/docs/internal/upstream/netplay-disabled-online-stubs.md). Consumers need a framework pin bump and rebuild; no regeneration.

## PR 489: release: preserve the .exe suffix when selecting Windows release binaries

**Current behavior:** Release discovery can select an unsuffixed executable alias and omit required Windows dependencies.
**Desired behavior:** Packaging preserves the `.exe` filename through dependency staging and signing.

Preserve the `.exe` suffix during Windows release executable discovery so dependency bundling and signing select the real executable.

Scope: game-agnostic packaging fix.

Validation: Windows GCC 13.2 Release; 30 bundled-release-layout tests passed.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/fix/windows-release-executable-suffix/docs/internal/upstream/windows-release-executable-suffix.md). Consumers need to repackage; no regeneration.

## PR 490: debug: add guest MIPS disassembly to the TCP server

**Current behavior:** The TCP server cannot inspect decoded instructions in live guest memory.
**Desired behavior:** A bounded command returns readable guest MIPS instructions.

Add a bounded `disasm` TCP command for live guest MIPS inspection.

Scope: game-agnostic debug feature.

Validation: arithmetic, branch/jump, COP0/COP2, unknown-word, and bounded-output tests passed; debug-tools ON/OFF builds passed.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/tcp-guest-disassembly/docs/internal/upstream/tcp-guest-disassembly.md).

## PR 491: debug: filter recorded write traces by producer PC

**Current behavior:** `wtrace_dump` cannot limit results to writes from a selected producer-PC range.
**Desired behavior:** Callers can filter trace results by inclusive/exclusive producer-PC bounds.

Add producer-PC bounds to `wtrace_dump`.

Scope: game-agnostic debug feature.

Validation: wrapped-ring filtering, PC boundaries, combined filters, and DMA metadata tests passed.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/wtrace-producer-pc-filter/docs/internal/upstream/wtrace-producer-pc-filter.md).

## PR 492: render passes: expose refusal and framebuffer failure details over TCP

**Current behavior:** Rejected render passes return zero without a usable reason.
**Desired behavior:** TCP diagnostics identify the refusal or framebuffer/checkpoint failure.

Report render-pass refusal and framebuffer failure details through `render_pass_stats`.

Scope: game-agnostic runtime diagnostics.

Validation: render-pass abort/sandbox tests passed; real OpenGL checks passed at 1x and 4x.

Limit: injected driver out-of-memory behavior was not tested.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/render-pass-refusal-diagnostics/docs/internal/upstream/render-pass-refusal-diagnostics.md).

## PR 493: render passes: restore mod callback context after watchdog aborts

**Current behavior:** A watchdog abort can leave stale mod callback depth and owner state.
**Desired behavior:** Render-pass rollback restores the interrupted callback context.

Restore mod callback depth and owner after render-pass watchdog aborts.

Scope: game-agnostic runtime fix.

Validation: nested callback, nonzero outer context, normal-return imbalance, and watchdog recovery tests passed.

Limit: depends on PR 492.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/fix/render-pass-mod-callback-rollback/docs/internal/upstream/render-pass-mod-callback-rollback.md).

## PR 494: gte: add opt-in projection-distance scaling

**Current behavior:** GTE projection distance is fixed to the guest-provided value.
**Desired behavior:** An opt-in setting scales projection distance while retaining canonical guest state by default.

Add opt-in GTE projection-distance scaling through `[video] fov_scale` and `PSX_GTE_FOV_SCALE`.

Scope: game-agnostic runtime configuration.

Validation: identity, projection arithmetic, invalid-value, environment parsing, and quantization tests passed.

Limit: the default remains identity.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/gte-projection-distance-scale/docs/internal/upstream/gte-projection-distance-scale.md). Consumers need a pin bump and rebuild; no regeneration.

## PR 495: input: add trusted offline controller sources for game plugins

**Current behavior:** Game plugins cannot provide controller state through the normal offline input path.
**Desired behavior:** Trusted plugins can contribute controller state without bypassing existing input rules.

Allow trusted game plugins to provide offline controller state at the existing input boundary.

Scope: game-agnostic plugin API; Medal of Honor consumes it.

Validation: absent, invalid, declined, neutralization, detach, and session-reset tests passed.

Integration: [Medal of Honor, SLUS-00974](https://github.com/FractalEngineer/Medal-Of-Honor-PSX-Recomp/tree/e323885958c2ae0b0df847bed6175f9e76524442) reached gameplay with tracked controller input. A pin bump and rebuild are required; no regeneration.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/offline-plugin-controller-sources/docs/internal/upstream/offline-plugin-controller-sources.md).

## PR 496: render passes: add atomic stereo pairs and scoped camera views

**Current behavior:** Render passes expose one image at a time and cannot scope a distinct rigid camera view per eye.
**Desired behavior:** Both eyes are rendered from one checkpoint and published only as a complete pair.

Add atomic paired-eye rendering and scoped rigid camera views for render-pass callbacks.

Scope: game-agnostic render-pass API; Medal of Honor consumes it.

Validation: pair recovery, pose math, projection, and real-OpenGL stereo checks passed. Connected Quest/VDXR stereo smoke passed.

Limit: depends on PRs 492-494; calibration and reconnect coverage remain open.

Integration: [Medal of Honor, SLUS-00974](https://github.com/FractalEngineer/Medal-Of-Honor-PSX-Recomp/tree/e323885958c2ae0b0df847bed6175f9e76524442) requires a pin bump and rebuild; no regeneration.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/atomic-stereo-render-views/docs/internal/upstream/atomic-stereo-render-views.md).

## PR 497: openxr: add an experimental Win32/OpenGL backend

**Current behavior:** The runtime has no native OpenXR presentation or tracked headset/controller pose backend.
**Desired behavior:** A default-off Win32/OpenGL backend submits fresh stereo pairs using runtime-tracked poses.

Add a default-off Win32/OpenGL OpenXR backend with headset and controller pose submission.

Scope: game-agnostic backend; currently Win32/OpenGL only.

Validation: XR/debug ON/OFF Release builds passed. Quest/VDXR reached a focused session with valid tracked eye, grip, and aim poses.

Limit: depends on PRs 495-496; reconnect, calibration, other runtimes, and standalone Quest are not covered.

Integration: [Medal of Honor, SLUS-00974](https://github.com/FractalEngineer/Medal-Of-Honor-PSX-Recomp/tree/e323885958c2ae0b0df847bed6175f9e76524442) requires a pin bump and rebuild; no regeneration.

[Source and attribution record](https://github.com/FractalEngineer/psxrecomp/blob/feat/experimental-openxr-win32-gl/docs/internal/upstream/experimental-openxr-win32-gl.md).
