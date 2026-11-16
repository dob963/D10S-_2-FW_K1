#include "calib.h"
#include "spur_reject.h"
static int16_t dc_offset[SPEC_BINS];
void calib_load(void){
    for(int i=0;i<SPEC_BINS;i++) dc_offset[i]=0;
}
int16_t calib_apply_dc(int16_t v,int bin){
    if(bin<0||bin>=SPEC_BINS) return v;
    int16_t out=v-dc_offset[bin];
    if(out<0) out=0;
    return out;
}
