#include "sweep_engine.h"
#include "bk4819_drv.h"
void sweep_adaptive(uint32_t f_start,uint32_t f_stop,
                    int16_t *spectrum_out,int16_t *afc_out){
    uint32_t span=f_stop-f_start;
    uint32_t step_coarse=span/(SPEC_BINS/4);
    uint32_t step_fine=span/SPEC_BINS;
    int idx=0; uint32_t f=f_start;
    while(f<f_stop&&idx<SPEC_BINS){
        bk4819_set_freq(f); bk4819_wait_settle();
        int16_t rssi=bk4819_read_rssi(), afc=bk4819_read_afc();
        spectrum_out[idx]=rssi; afc_out[idx]=afc; idx++;
        uint32_t step=(rssi<60)?step_coarse:step_fine; f+=step;
    }
    while(idx<SPEC_BINS){spectrum_out[idx]=0;afc_out[idx]=0;idx++;}
}
void sweep_average(const int16_t *in,int16_t *acc,int n,int alpha_q8){
    int beta=256-alpha_q8;
    for(int i=0;i<n;i++){
        int32_t v=((int32_t)in[i]*alpha_q8+(int32_t)acc[i]*beta)>>8;
        acc[i]=(int16_t)v;
    }
}
