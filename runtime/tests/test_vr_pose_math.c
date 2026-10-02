#include "vr_pose_math.h"
#include <math.h>
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void) {
    const double q[4]={0,0,0,1},p[3]={-.0335,0,0},origin[3]={0,0,0};
    const double f[4]={-atan(1),atan(1),atan(.5),-atan(.5)};
    PSXModRenderView v;
    CHECK(vr_pose_to_view(q,p,q,origin,f,1000,512,240,&v));
    CHECK(v.translation[0]==34 && v.translation[1]==0 && v.translation[2]==0);
    CHECK(v.rotation_q12[0]==4096 && v.rotation_q12[4]==4096 && v.rotation_q12[8]==4096);
    CHECK(v.fx_q16==256*65536 && v.fy_q16==240*65536);
    CHECK(v.cx_delta_q16==0 && v.cy_delta_q16==0);
    double move[3]={0,.1,-.2};
    CHECK(vr_pose_to_view(q,move,q,origin,f,1000,512,240,&v));
    CHECK(v.translation[1]==100 && v.translation[2]==-200);
    const double yaw[4]={0,sin(.2),0,cos(.2)};
    CHECK(vr_pose_to_view(yaw,origin,q,origin,f,1000,512,240,&v));
    CHECK(v.rotation_q12[2]>0 && v.rotation_q12[6]<0);
    CHECK(vr_pose_to_view(yaw,move,yaw,move,f,1000,512,240,&v));
    CHECK(v.rotation_q12[0]==4096 && v.translation[0]==0 && v.translation[2]==0);
    double asymmetric[4]={-atan(.8),atan(1.2),atan(.7),-atan(.3)};
    CHECK(vr_pose_to_view(q,origin,q,origin,asymmetric,1000,512,240,&v));
    CHECK(v.cx_delta_q16==-3355443 && v.cy_delta_q16==3145728);
    double bad[4]={0,0,0,0};CHECK(!vr_pose_to_view(bad,p,q,origin,f,1000,512,240,&v));
    CHECK(!vr_pose_to_view(q,p,q,origin,f,NAN,512,240,&v));
    puts("PASS: metric IPD, axis signs, recenter, rotation and asymmetric FOV");return 0;
}
