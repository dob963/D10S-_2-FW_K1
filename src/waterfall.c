#include "waterfall.h"
#include "spur_reject.h"

static const uint16_t PAL[16] = {
    0x0000,0x0800,0x1020,0x2040,0x3040,0x4040,0x5040,0x6040,
    0x7040,0x8040,0x9040,0xA040,0xB040,0xC040,0xD040,0xF800
};

static uint8_t wf[16][SPEC_BINS];
static uint8_t wf_enabled = 1u;

void waterfall_set_enabled(int enabled)
{
    wf_enabled = enabled ? 1u : 0u;
}

int waterfall_is_enabled(void)
{
    return wf_enabled != 0u;
}

void waterfall_clear(void)
{
    for (int r = 0; r < 16; ++r)
        for (int i = 0; i < SPEC_BINS; ++i)
            wf[r][i] = 0u;
}

void waterfall_push(const int8_t *spec, int n)
{
    if (!wf_enabled || spec == 0 || n <= 0) return;
    if (n > SPEC_BINS) n = SPEC_BINS;

    for (int r = 15; r > 0; --r)
        for (int i = 0; i < SPEC_BINS; ++i)
            wf[r][i] = wf[r - 1][i];

    for (int i = 0; i < n; ++i) {
        int v = spec[i] < 0 ? 0 : spec[i];
        wf[0][i] = (uint8_t)(v >> 3);
    }

    (void)PAL;
}
