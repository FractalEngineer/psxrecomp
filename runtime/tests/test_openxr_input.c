#include "psx_openxr.h"
#include <assert.h>
#include <math.h>
int main(void) {
    PSXModOpenXRInput p={0},s={0};
    p.struct_size=sizeof p;
    assert(psx_openxr_input(&p) && !p.focused && p.stick[0][0]==0 && !p.synthetic);
    s.struct_size=sizeof s;s.focused=1;s.active[0]=s.active[1]=1;
    s.stick[0][0]=-.75f;s.stick[0][1]=1;s.stick[1][0]=.6f;
    assert(psx_openxr_input_override(&s));
    assert(psx_openxr_input(&p) && p.synthetic && p.stick[0][0]==-.75f && p.stick[1][0]==.6f);
    uint64_t seq=p.sequence;psx_openxr_input_snapshot(&p);assert(p.sequence==seq);
    s.focused=0;assert(psx_openxr_input_override(&s));
    assert(psx_openxr_input(&p) && p.synthetic && !p.focused && p.stick[0][1]==0 && p.stick[1][0]==0);
    s.focused=1;s.active[0]=0;assert(psx_openxr_input_override(&s));
    assert(psx_openxr_input(&p) && p.stick[0][0]==0 && p.stick[1][0]==.6f);
    s.stick[0][0]=NAN;assert(!psx_openxr_input_override(&s));
    s.stick[0][0]=1.1f;assert(!psx_openxr_input_override(&s));
    assert(psx_openxr_input_override(0));
    assert(psx_openxr_input(&p) && !p.synthetic && p.stick[1][0]==0);
    s.stick[0][0]=1;assert(psx_openxr_input_override(&s));
    psx_openxr_shutdown();assert(psx_openxr_input(&p) && !p.synthetic && !p.focused);
    return 0;
}
