#ifndef PSX_GTE_VIEW_H
#define PSX_GTE_VIEW_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Host enhancement ambient, checkpointed by render transactions. Identity
 * outside an explicitly scoped render callback; never writes guest TR regs. */
void gte_render_view_get(int32_t xyz[3]);
void gte_render_view_set(const int32_t xyz[3]);
#ifdef __cplusplus
}
#endif
#endif
