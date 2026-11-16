#include "ui_spectrum.h"
static int8_t peak[SPEC_BINS], prev_disp[SPEC_BINS];
void ui_spectrum_init(void){
    for(int i=0;i<SPEC_BINS;i++){peak[i]=0;prev_disp[i]=0;}
}
void ui_spectrum_clamp_gradient(int8_t *spec,int n,int max_delta){
    for(int i=1;i<n;i++){
        int d=spec[i]-spec[i-1];
        if(d>max_delta)spec[i]=spec[i-1]+max_delta;
        if(d<-max_delta)spec[i]=spec[i-1]-max_delta;
    }
}
void ui_spectrum_crossfade(const int8_t *new_spec,int n,int alpha_q8){
    int beta=256-alpha_q8;
    for(int i=0;i<n;i++){
        int32_t v=((int32_t)new_spec[i]*alpha_q8+(int32_t)prev_disp[i]*beta)>>8;
        if(v<0)v=0; if(v>255)v=255;
        prev_disp[i]=(int8_t)v;
    }
}
void ui_spectrum_draw(const int8_t *spec,int n){
    ui_spectrum_crossfade(spec,n,96);
    ui_spectrum_clamp_gradient(prev_disp,n,24);
    for(int i=0;i<n&&i<SPEC_BINS;i++){
        int8_t v=prev_disp[i];
        if(v>peak[i])peak[i]=v; else if(peak[i]>0)peak[i]--;
        /* TODO rendering display V3 */
    }
}
int ui_spectrum_poll_event(void){return 0;}
void ui_spectrum_handle(void){}
