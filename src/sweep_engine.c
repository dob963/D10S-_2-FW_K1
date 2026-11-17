#include "sweep_engine.h"
#include "bk4829_drv.h"

void sweep_adaptive(uint32_t f_start, uint32_t f_stop,
                    int16_t *spectrum_out, int16_t *afc_out,
                    uint8_t *valid_out)
{
    if (!spectrum_out || !afc_out || !valid_out) return;
    for (int i = 0; i < SPEC_BINS; ++i) {
        spectrum_out[i] = 0;
        afc_out[i] = 0;
        valid_out[i] = 0u;
    }
    if (f_stop <= f_start) return;

    uint32_t span = f_stop - f_start;
    uint32_t step_coarse = span / (SPEC_BINS / 4u);
    uint32_t step_fine = span / SPEC_BINS;
    if (step_coarse == 0u) step_coarse = 1u;
    if (step_fine == 0u) step_fine = 1u;

    uint32_t f = f_start;
    uint32_t samples = 0u;
    while (f < f_stop && samples < SPEC_BINS) {
        bk4829_set_freq(f);
        bk4829_wait_settle();
        int16_t rssi = bk4829_read_rssi();
        int16_t afc = bk4829_read_afc();

        /* Mappa la misura nel bin della frequenza realmente campionata.
         * In modalità coarse restano buchi espliciti: non comprimere i
         * campioni nei primi 32 indici e non interpolare picchi non misurati.
         */
        uint32_t bin = (uint32_t)((((uint64_t)(f - f_start) * SPEC_BINS) + (span / 2u)) / span);
        if (bin >= SPEC_BINS) bin = SPEC_BINS - 1u;
        spectrum_out[bin] = rssi;
        afc_out[bin] = afc;
        valid_out[bin] = 1u;
        ++samples;

        uint32_t step = (rssi < 60) ? step_coarse : step_fine;
        if (step > (f_stop - f)) break;
        f += step;
    }
}

void sweep_average(const int16_t *in, int16_t *acc, int n, int alpha_q8)
{
    int beta = 256 - alpha_q8;
    for (int i = 0; i < n; ++i) {
        int32_t v = ((int32_t)in[i] * alpha_q8 +
                     (int32_t)acc[i] * beta) >> 8;
        acc[i] = (int16_t)v;
    }
}
