#include "stabilizer.h"
#include <string.h>
void stabilizer_init(stabilizer_t *s){ memset(s,0,sizeof(*s)); }
void stabilizer_apply(stabilizer_t *s,const int8_t *in,int8_t *out,int n){
    for(int i=0;i<n&&i<SPEC_BINS;i++){
        int v_in=in[i]<0?0:in[i];
        int v_sh=s->shown[i];
        int diff=v_in-v_sh;
        if(diff>-STAB_DEAD && diff<STAB_DEAD){ /* deadband */ }
        else if(diff>0){
            if(diff>=STAB_HI){ s->shown[i]=(int8_t)v_in; s->hold[i]=STAB_HOLD; }
        } else {
            if(s->hold[i]>0){ s->hold[i]--; }
            else if(-diff>=STAB_LO){
                int nv=v_sh-STAB_RELEASE;
                if(nv<v_in) nv=v_in;
                if(nv<0) nv=0;
                s->shown[i]=(int8_t)nv;
            }
        }
        out[i]=s->shown[i];
    }
}
