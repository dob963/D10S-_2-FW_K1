#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "peak_tracker.h"
int main(void){
 peak_tracker_t state; peak_tracker_result_t r; int8_t a[SPEC_BINS]={0};
 peak_tracker_init(&state); for(int i=0;i<SPEC_BINS;++i)a[i]=20;
 a[63]=24;a[64]=70;a[65]=30;
 peak_tracker_update(&state,a,SPEC_BINS,&r); assert(r.noise_floor==20); assert(r.peak_bin==-1);
 peak_tracker_update(&state,a,SPEC_BINS,&r); assert(r.peak_bin==64); assert(r.peak_level==70); assert(r.peak_offset_q8>0);
 peak_tracker_init(&state); for(int i=0;i<SPEC_BINS;++i)a[i]=20; a[64]=100;
 peak_tracker_update(&state,a,SPEC_BINS,&r); assert(r.peak_bin==-1);
 puts("Peak tracker tests: PASS"); return 0;
}
