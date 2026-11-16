#include "spur_reject.h"
#include <string.h>
void spur_init(spur_t *s){memset(s,0,sizeof(*s));}
void spur_update(spur_t *s,const int16_t *spectrum,
                 const int16_t *afc,int8_t *out_clean){
    for(int i=0;i<SPEC_BINS;i++){
        int16_t v=spectrum[i];
        int16_t d_afc=afc[i]-s->prev_afc[i]; if(d_afc<0)d_afc=-d_afc;
        int16_t d_amp=v-s->prev_amp[i];      if(d_amp<0)d_amp=-d_amp;
        int stable_afc=(d_afc<=AFC_TOL), stable_amp=(d_amp<=AMP_TOL);
        int strong=(v>120);
        if(strong&&stable_afc&&stable_amp){
            if(s->persist[i]<PERSIST_N)s->persist[i]++;
        } else {
            if(s->persist[i]>0)s->persist[i]--;
        }
        s->spur_flag[i]=(s->persist[i]>=PERSIST_N);
        int16_t clean=v;
        if(s->spur_flag[i]){clean=v-(v>>1);if(clean<0)clean=0;}
        out_clean[i]=(int8_t)clean;
        s->prev_afc[i]=afc[i]; s->prev_amp[i]=v;
    }
}
