#include "spur_reject.h"
#include "sweep_engine.h"
#include "bk4829_drv.h"
#include "median3.h"
#include "waterfall.h"
#include "ui_spectrum.h"
#include "stabilizer.h"
#include "hw_init.h"
#include "calib.h"
#include "st7565.h"

static int16_t spec_raw[SPEC_BINS], spec_avg[SPEC_BINS];
static int16_t afc_raw[SPEC_BINS];
static uint8_t spec_valid[SPEC_BINS];
static int8_t spec_clean[SPEC_BINS], spec_stable[SPEC_BINS];
static stabilizer_t stab;

int main(void)
{
    hw_init();
    calib_load();
    stabilizer_init(&stab);
    ui_spectrum_init();
    ui_spectrum_set_info(146000000u, 4000000u, 0u);

    /* Banda iniziale; la UI di configurazione non è ancora implementata. */
    const uint32_t f_start = 144000000u;
    const uint32_t f_stop  = 148000000u;

    for (;;) {
        sweep_adaptive(f_start, f_stop, spec_raw, afc_raw, spec_valid);

        /* Tutti i bin sono misurati dallo sweep uniforme. La mediana usa
         * i tre bin adiacenti reali, senza compattare indici né creare
         * adiacenze artificiali tra frequenze distanti.
         */
        for (int i = 0; i < SPEC_BINS; ++i)
            spec_avg[i] = spec_raw[i];

        for (int i = 1; i < SPEC_BINS - 1; ++i) {
            if (spec_valid[i - 1] && spec_valid[i] && spec_valid[i + 1])
                spec_avg[i] = median3(spec_raw[i - 1],
                                      spec_raw[i],
                                      spec_raw[i + 1]);
        }

        for (int i = 0; i < SPEC_BINS; ++i) {
            if (!spec_valid[i]) {
                spec_clean[i] = 0;
                continue;
            }

            /* BK4829 RSSI raw è 9 bit. La conversione upstream è
             * dBm = raw/2 - 160; per il grafico usiamo la stessa scala
             * relativa a -160 dBm, compressa a 7 bit (raw/4).
             * Un guadagno software variabile per campione è escluso:
             * alterava la relazione d'ampiezza tra bin consecutivi.
             */
            int32_t level = (spec_avg[i] > 0) ? (spec_avg[i] / 4) : 0;
            level = calib_apply_dc((int16_t)level, i);
            if (level > 127) level = 127;
            spec_clean[i] = (int8_t)level;
        }

        /* AFC non è validato: non applicare spur rejection basato su
         * misure AFC fittizie. Il filtro mediano limita gli outlier isolati.
         */
        stabilizer_apply(&stab, spec_clean, spec_stable, SPEC_BINS);
        ui_spectrum_draw(spec_stable, SPEC_BINS);
        waterfall_push(spec_clean, SPEC_BINS);
        st7565_blit_full();

        delay_ms(67u);
        if (ui_spectrum_poll_event()) ui_spectrum_handle();
    }
}
