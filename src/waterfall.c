#include "waterfall.h"
#include "spur_reject.h"
#include "st7565.h"

#define WF_ROWS 16
#define WF_TOP  16

static uint8_t wf[WF_ROWS][SPEC_BINS];
static uint8_t wf_enabled = 1u;

/* Matrice Bayer 4x4 per simulare 16 livelli in monocromia. */
static const uint8_t bayer4[4][4] = {
    { 0, 8, 2,10 }, {12, 4,14, 6 },
    { 3,11, 1, 9 }, {15, 7,13, 5 }
};

void waterfall_set_enabled(int enabled) { wf_enabled = enabled ? 1u : 0u; }
int waterfall_is_enabled(void) { return wf_enabled != 0u; }

void waterfall_clear(void)
{
    for (int r=0;r<WF_ROWS;++r)
        for (int i=0;i<SPEC_BINS;++i) wf[r][i]=0u;
}

void waterfall_push(const int8_t *spec, int n)
{
    if (!wf_enabled || !spec || n<=0) return;
    if (n>SPEC_BINS) n=SPEC_BINS;

    for (int r=WF_ROWS-1;r>0;--r)
        for (int i=0;i<SPEC_BINS;++i) wf[r][i]=wf[r-1][i];

    for (int i=0;i<n;++i) {
        int v=spec[i]<0 ? 0 : spec[i];
        if (v>255) v=255;
        wf[0][i]=(uint8_t)((v * 16) / 127);
    }

    /* Render separato: 16 righe in alto, nessun effetto sulla traccia live.
     * La risoluzione orizzontale viene rimappata a 128 pixel.
     */
    for (int y=0;y<WF_ROWS;++y) {
        for (int x=0;x<ST7565_WIDTH;++x) {
            int bin=(x*n)/ST7565_WIDTH;
            if (bin>=n) bin=n-1;
            uint8_t level=wf[y][bin];
            uint8_t threshold=bayer4[y&3][x&3];
            st7565_set_pixel(x,WF_TOP+y,level>threshold);
        }
    }
}
