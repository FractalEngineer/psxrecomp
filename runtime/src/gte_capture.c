#include "gte_capture.h"

static int            g_enabled = 0;
static GteCaptureSink g_sink    = 0;
static void*          g_user    = 0;

void gte_capture_enable(int enabled) { g_enabled = enabled ? 1 : 0; }

int gte_capture_enabled(void) { return g_enabled; }

void gte_capture_set_sink(GteCaptureSink sink, void* user) {
    g_sink = sink;
    g_user = user;
}

void gte_capture_vertex(const GteCaptureVertex* vertex) {
    if (g_enabled && g_sink) {
        g_sink(vertex, g_user);
    }
}
