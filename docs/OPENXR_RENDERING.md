# Experimental OpenXR rendering

Build with `-DPSX_OPENXR=ON` (default OFF). This opt-in Win32/OpenGL backend
fetches the official OpenXR SDK 1.1.54 at pinned commit
`b76b80adaf65ac3ad6cc1ce61974fb29a5d02352` and links its static loader.
The SDK is absent from ordinary builds. `PSX_OPENXR=1` in the launched process
requests a GL 4.6 context; ordinary launches retain GL 3.3. A game plugin must
separately enable the API and provide a draw-only scene callback.

## Contract

`psx_mod_openxr_begin(w,h,units_per_meter)` handles events and locates both eyes
at one predicted display time in LOCAL space. Retrieve each
`psx_mod_openxr_view(eye,&view)` and use `psx_mod_render_view` inside the paired
render callback. End with `psx_mod_openxr_end(rendered)`. Only a newly published
pair may be submitted. Failed/shed redraws end with zero layers; a desktop pair
from an older pose is never submitted with a new pose. Camera transformation is
inverse eye orientation relative to the recentered game view, with metric eye
translation converted into game camera units. XR Y-up/Z-back becomes PSX
Y-down/Z-forward. Projection uses each eye's actual asymmetric FOV.

Guest registers remain untouched by host configuration. Transactions checkpoint
and restore the entire host render pose, including rotations and projection,
on both ordinary return and watchdog abort. The exact identity path preserves
canonical GTE arithmetic. Optional `projection_h_ref` scales the focal terms
by the guest's effective H divided by the reference H. This preserves a game's
separate viewmodel focal length without changing world draws at the reference H.
It is game policy, not an OpenXR requirement; other scenes need verification.

XR images use independent swapchains at runtime-recommended sizes. Acquire,
wait, blit the complete eye, release, and submit both projection views with
the same poses/FOV/time used for rendering. The copy flips Y at the XR boundary;
the user measured inverted output with the original direct blit, and confirmed
the corrected headset image. Context teardown destroys XR resources before GL.
Explicit re-enable retries initialization; unsupported/disconnected initialization
stays inspectable instead of repeatedly allocating. Enable changes are refused
while a frame/eye is active. Reference-space changes recenter at their change time.

## Inspection

- `openxr_stats`: session/tracking state, actual IPD, requested units/meter,
  frame/submission/failure counts, actual and required GL versions, last XR error.
- `openxr_views`: latest located metric poses/FOV and derived render matrices,
  plus last submitted pair/cycle/display-time identifiers. Latest located data
  can differ from last submitted data while an eye or a shed frame is pending.
- `openxr_control recenter=1`: make the next located head position/orientation
  the game view origin. `enable=0|1` changes/retries headset initialization.
- `gte_ring_dump render=0|1 offset=N count=N frame=F`: inspect native versus
  host-view projections, with pagination and count up to 4096. Each record's
  render_view tag records an active full pose or nonzero legacy view offset.

## Measured scope and limitations

Quest 3 with VirtualDesktopXR accepted real projection submissions. Actual eye
separation sampled 0.066780m. The runtime required GL >=4.0; the initial 3.3
context was refused, and the opt-in 4.6 context succeeded. User confirmed upright
images and expected head-tracking direction after the copy correction. Release
samples at game internal scale 5 completed hundreds of pairs without shedding;
Debug's measured cost exceeded its existing budget and produced empty XR frames.
These are cadence samples, not motion-to-photon or headset throughput results.

This first path still draws and waits from the game's draw boundary (30Hz for
Medal of Honor). It does not provide an independent headset-rate replay pump,
wrist HUD, controller poses/aiming, compositor depth or late head-pose reprojection.
Unsubmitted images are held/reprojected by runtime policy. Native game culling
may omit geometry revealed by wider FOV or head turns. Metric scale and per-domain
weapon/HUD policies remain game-owned. Physical world scale is not established
by the successful session or subjective tuning.

## Opt-in locomotion input

The backend creates a vector2 thumbstick action with left/right hand subactions,
suggests the Oculus Touch interaction profile and attaches its action set before
session start. `psx_mod_openxr_input` synchronizes actions at normal offline
host input sampling, independently of eye rendering. Its fresh sample includes
focus, per-hand activity, XR stick coordinates (positive Y forward/up), sequence
and a synthetic tag. Initialization failure, session loss, unfocused sync and
inactive actions produce neutral axes. An eye transaction refuses input polling.
Focus handling follows the [OpenXR xrSyncActions contract](https://registry.khronos.org/OpenXR/specs/1.1/man/html/xrSyncActions.html).

`psx_mod_set_controller_source(player,callback)` installs a trusted game-owned
offline source. It supplies the complete pad axes/type and an active-low button
word; the frontend merges local button words for menus and delivers through its
existing coherent SIO type-request/selfcheck path. Invalid/declined samples and
detach release axes. The existing post-load guard neutralizes the result. TCP
input overrides retain priority. Netplay/resim and eye replay do not call it.
Sources reset before every mod-session activation. No source is installed by
default; game policy decides mappings, deadzone, gain and whether to use it.
Low-latency frontend refresh can sample a second time within one VBlank.

TCP `openxr_input` observes the last sample without polling or consuming it.
`pad_status` observes current SIO delivery. Debug-only `openxr_input_override`
injects signed-thousandth lx/ly/rx/ry, focused and left_active/right_active for
desktop controls; `clear=1` releases it. Injected samples always say synthetic=1.
These are separate action/pad queries, not an atomic producer/delivery record.

Validation includes compiled-out input tests, source validation/release/reset
tests, real Debug/Release game builds and game-owned mapping tests. Medal of
Honor slot-3 synthetic movement/turn controls release correctly; neutral stereo
on/off controls match all 96 measured guest fingerprint rows. Real Quest/VDXR
action samples have both hands active with positive/negative axes, and a bounded
Release sample submits 3,131 frames without XR failures or stereo shedding.
Detailed receipts and user direction assessment live in the game docs/reverse.
The game's first linear-byte mapper needed live native-curve compensation and
a radial movement deadzone; after correction the user confirmed consistent
movement. This game-specific conversion does not belong in the XR backend.
These results do not establish headset-rate scheduling or calibrated turn speed.

Sources: [OpenXR specification](https://registry.khronos.org/OpenXR/specs/1.0-khr/html/xrspec.html),
[official SDK](https://github.com/KhronosGroup/OpenXR-SDK),
[VDXR supported features](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki/Developers).

## Touch combat inputs (2026-10-02)

The same opt-in action set now includes float trigger/squeeze values and boolean
primary, secondary, Menu and thumbstick-click actions. Touch X/A use PRIMARY=1,
Y/B SECONDARY=2, left Menu=4 and stick click=8. Only the left Menu binding is
suggested. Trigger and squeeze are [0,1] with independent active flags; clicks
have separate active and pressed masks. `active[]` continues to mean thumbstick
activity only. Buttons do not disappear solely because a stick is inactive.
Mapping and analog thresholds belong to the game, not the framework.
Bindings follow the [Khronos Touch interaction profile](https://registry.khronos.org/OpenXR/specs/1.0-khr/html/xrspec.html#_oculus_touch_controller_profile).

PSXModOpenXRInput has appended fields; its struct_size contract still requires
the current sizeof value. Consumers must rebuild against the updated header.
Unavailable/unfocused/failed samples are fresh neutral values for every action;
click values are intersected with action activity. Eye replay still never polls
input. The backend suggests 13 bindings and attaches before session start.

TCP openxr_input includes trigger/squeeze, their activity, and buttons plus
buttons_active. Synthetic override accepts left_trigger/right_trigger and
left_squeeze/right_squeeze in thousandths [0,1000], corresponding *_active flags
0|1, and left_buttons/right_buttons plus *_active click masks [0,15]. These
activity flags are independent of left_active/right_active (sticks). Default
synthetic click/analog activity is enabled, with zero values; clear=1 releases.
Overrides remain labelled synthetic=1 and validation is performed before change.

Desktop tests exercise independent activity, invalid values, focus/clear/shutdown
release and actual game-owned native ammo/stance writers. The Quest 3/VDXR user
subsequently confirmed all mapped combat controls. A separate 246-row actual
input sample (synthetic=0) caught right squeeze reaching 1.0 with adjacent native
R2 delivery, then unfocused neutral samples. It did not catch the other presses;
their hardware acceptance is user feedback, not trace proof. Its trailing status
queries failed after the bounded game closed. The pause menu is visible but too
close; game aiming still follows the native body, without controller poses.
Controlled hardware focus-loss/reconnection and headset cadence remain open.

## Tracked controller poses (2026-10-02)

`psx_mod_openxr_hands` returns a separate struct-size-checked snapshot; the input
ABI is unchanged. Four action spaces cover left/right grip/aim, bringing the
Touch binding suggestions to 17. Spaces are located in LOCAL at the same
predicted display time as the eyes, after establishing their recenter origin.
The getter does not synchronize actions or locate spaces, including during
eye replay. Raw positions are meters and quaternions are xyzw; the snapshot
includes the eye recenter origin, focus, per-pose activity, OpenXR validity and
tracking flags, sequence, predicted time and host sample age. No real frame
reports age UINT32_MAX. Recenter, unsuccessful new frames, unfocused input and
shutdown clear cached poses. Partial poses retain flags but have zero values;
consumers must require both validity bits, activity, focus and a fresh sample.

`vr_pose_to_transform` computes the recentered PSX pose transform, inverse to
the existing view transform: R=S O^T E S, t=units S O^T(p-origin), with
S=diag(1,-1,-1). It does not select game world/body coordinates or weapon policy.
Game code owns those decisions and its stale-sample threshold.

TCP `openxr_hands` observes the snapshot; debug-only `openxr_hands_override`
injects explicitly synthetic poses using mm positions and millionth quaternion
components, with focus/activity/flags and clear controls. Validation precedes
mutation. Desktop controls passed read-only synthetic sequence checks, rejected
quaternion non-mutation, partial validity, unfocused clearing and explicit
clear. Compiled-out lifecycle tests and pose inverse/axis tests passed; the
SDK-enabled Release game builds. Real Quest action-space binding, alignment,
tracking-loss behavior and controller-driven game aiming remain unverified.

Contract references: [action pose state](https://registry.khronos.org/OpenXR/specs/1.0/man/html/xrGetActionStatePose.html),
[Khronos spaces specification](https://github.com/KhronosGroup/OpenXR-Docs/blob/main/specification/sources/chapters/spaces.adoc).
