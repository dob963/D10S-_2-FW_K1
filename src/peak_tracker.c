#include "peak_tracker.h"
static void sort_u8(uint8_t *a,int n)
{
    for(int i=1;i<n;++i) { uint8_t v=a[i]; int j=i; while(j>0&&a[j-1]>v){a[j]=a[j-1];--j;} a[j]=v; }
}
void peak_tracker_init(peak_tracker_t *s)
{
    if(!s)return;
    for(int i=0;i<SPEC_BINS;++i)s->previous[i]=0u;
}
void peak_tracker_update(peak_tracker_t *s,const int8_t *levels,int n,peak_tracker_result_t *r)
{
    if(!s||!levels||!r||n<=0)return;
    if(n>SPEC_BINS)n=SPEC_BINS;
    uint8_t sorted[SPEC_BINS],candidate[SPEC_BINS]={0};
    for(int i=0;i<n;++i){int v=levels[i];sorted[i]=(uint8_t)(v<0?0:(v>127?127:v));}
    sort_u8(sorted,n);
    uint8_t floor=sorted[((n-1)*20)/100];
    uint16_t threshold=(uint16_t)floor+8u;
    if(threshold>127u)threshold=127u;
    int best=-1; uint8_t best_level=0u; int16_t best_offset=0;
    for(int i=0;i<n;++i){
        int v=levels[i]; if(v<0)v=0; if(v>127)v=127;
        int left=(i>0)?levels[i-1]:-1, right=(i+1<n)?levels[i+1]:-1;
        if(left<0)left=0;
        if(right<0)right=0;
        if(left>127)left=127;
        if(right>127)right=127;
        int neighbor=(left>right)?left:right;
        if((uint16_t)v<threshold||v-neighbor<4||v<left||v<right)continue;
        candidate[i]=1u;
        int confirmed=0;
        for(int k=i-1;k<=i+1;++k)if(k>=0&&k<n&&s->previous[k])confirmed=1;
        if(confirmed&&v>best_level){
            best=i; best_level=(uint8_t)v;
            int denom=left-2*v+right,offset=0;
            if(denom<0){offset=((left-right)*128)/denom;if(offset>128)offset=128;if(offset< -128)offset=-128;}
            best_offset=(int16_t)offset;
        }
    }
    for(int i=0;i<SPEC_BINS;++i)s->previous[i]=(i<n)?candidate[i]:0u;
    r->noise_floor=floor; r->peak_level=best_level; r->peak_offset_q8=best_offset; r->peak_bin=(int16_t)best;
}
