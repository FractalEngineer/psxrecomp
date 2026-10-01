#ifndef PSX_GTE_CAPTURE_H
#define PSX_GTE_CAPTURE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One vertex observed at the GTE RTPS/RTPT seam (gte_rtps_internal).
 *
 * This is the ONLY place in psxrecomp where a camera-space vertex exists. The
 * renderers are 2-D VRAM rasterizers that only ever see already-projected
 * integer SXY/SZ, so true stereo needs the geometry captured here rather than
 * anywhere in a renderer.
 *
 *   cam_* = GTE MAC1/2/3 after the RT/TR matrix multiply (post-modelview,
 *           pre-perspective), in the guest's fixed-point world units.
 *   scr_* = projected SXY (signed 16-bit screen coords) for the same vertex.
 *   sz    = SZ3 depth used by the perspective divide.
 *   instr = the COP2 command word (RTPS 0x0008001 / RTPT 0x00080030 ...),
 *           so a sink can distinguish single vs triple.
 */
typedef struct GteCaptureVertex {
    int32_t  cam_x, cam_y, cam_z;
    int32_t  scr_x, scr_y;
    int32_t  sz;
    uint32_t instr;
} GteCaptureVertex;

typedef void (*GteCaptureSink)(const GteCaptureVertex* vertex, void* user);

/* Off by default. While disabled the seam costs one predictable branch. */
void gte_capture_enable(int enabled);
int  gte_capture_enabled(void);

/* Install or clear (NULL) the sink. Set once at startup. */
void gte_capture_set_sink(GteCaptureSink sink, void* user);

/* Called from gte_rtps_internal so the seam has a single dispatch point. */
void gte_capture_vertex(const GteCaptureVertex* vertex);

#ifdef __cplusplus
}
#endif

#endif /* PSX_GTE_CAPTURE_H */
