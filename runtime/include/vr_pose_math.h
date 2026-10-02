#pragma once
#include "mod_plugins.h"
/* XR right-handed X right/Y up/Z back; PSX X right/Y down/Z forward. */
int vr_pose_to_view(const double q[4], const double p[3],
                    const double origin_q[4], const double origin_p[3],
                    const double fov[4], double units_per_meter,
                    int width, int height, PSXModRenderView *out);
