#include "sweep_engine.h"
#include "bk4829_drv.h"

/*
 * Live spectrum: campionamento uniforme e completo.
 * L'algoritmo adattivo precedente saltava i bin e poteva terminare prima
 * di raggiungere f_stop quando incontrava segnali forti. In uno sweep
 * destinato alla visualizzazione ogni bin deve avere una misura reale.
 */
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

    if (f_stop <= f_start || SPEC_BINS < 2) return;

    const uint32_t span = f_stop - f_start;
    for (int i = 0; i < SPEC_BINS; ++i) {
        /* Interpolazione intera con arrotondamento: include entrambi gli
         * estremi e non accumula errore di passo lungo la banda.
         */
        const uint64_t numerator = (uint64_t)span * (uint32_t)i;
        const uint32_t freq = f_start +
            (uint32_t)((numerator + (SPEC_BINS - 1u) / 2u) /
                       (SPEC_BINS - 1u));

        bk4829_set_freq(freq);
        bk4829_wait_settle();

        /* Il driver scarta già il primo RSSI dopo il retune.
         * AFC rimane esplicitamente non validato e non viene usato.
         */
        const int16_t raw = bk4829_read_rssi();
        spectrum_out[i] = raw;
        afc_out[i] = 0;
        valid_out[i] = 1u;
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
