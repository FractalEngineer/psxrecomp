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
controller actions, wrist HUD, compositor depth or late head-pose reprojection.
Unsubmitted images are held/reprojected by runtime policy. Native game culling
may omit geometry revealed by wider FOV or head turns. Metric scale and per-domain
weapon/HUD policies remain game-owned. Physical world scale is not established
by the successful session or subjective tuning.

Sources: [OpenXR specification](https://registry.khronos.org/OpenXR/specs/1.0-khr/html/xrspec.html),
[official SDK](https://github.com/KhronosGroup/OpenXR-SDK),
[VDXR supported features](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki/Developers).
