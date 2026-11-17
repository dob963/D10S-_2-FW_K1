#include "agc_sw.h"
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
static int16_t spec_med_in[SPEC_BINS], afc_raw[SPEC_BINS];
static int8_t spec_clean[SPEC_BINS], spec_stable[SPEC_BINS];
static agc_t agc;
static spur_t spur;
static stabilizer_t stab;

int main(void)
{
    hw_init();
    calib_load();
    agc_init(&agc);
    spur_init(&spur);
    stabilizer_init(&stab);
    ui_spectrum_init();
    ui_spectrum_set_info(146000000u, 4000000u, 0u);

    /* TODO: rendere start/stop configurabili dalla UI. */
    const uint32_t f_start = 144000000u;
    const uint32_t f_stop  = 148000000u;

    for (int i = 0; i < SPEC_BINS; ++i) spec_avg[i] = 0;

    for (;;) {
        sweep_adaptive(f_start, f_stop, spec_raw, afc_raw);
        /* Traccia live: copia diretta dello sweep corrente.
         * sweep_average() non viene usata qui: introdurrebbe cross-fade.
         */
        for (int i = 0; i < SPEC_BINS; ++i)
            spec_avg[i] = spec_raw[i];

        /* Snapshot: la mediana legge sempre i campioni dello stesso sweep.
         * Evita che i risultati gia' filtrati contaminino i vicini successivi.
         * median3() resta invariata.
         */
        for (int i = 0; i < SPEC_BINS; ++i)
            spec_med_in[i] = spec_avg[i];
        for (int i = 1; i < SPEC_BINS - 1; ++i)
            spec_avg[i] = median3(spec_med_in[i - 1],
                                  spec_med_in[i],
                                  spec_med_in[i + 1]);

        for (int i = 0; i < SPEC_BINS; ++i)
            spec_avg[i] = agc_process(&agc, spec_avg[i]);
        for (int i = 0; i < SPEC_BINS; ++i)
            spec_avg[i] = calib_apply_dc(spec_avg[i], i);

        /* Contratto numerico dell'uscita DSP/display: 0..127.
         * spur_update() mantiene la sua API int8_t originale. Saturiamo
         * l'ingresso a 127 nell'adattatore, evitando conversioni signed
         * fuori intervallo e mantenendo coerenti spur threshold e stabilizer.
         * Il valore AGC interno resta 0..255 fino a questo punto.
         */
        for (int i = 0; i < SPEC_BINS; ++i) {
            if (spec_avg[i] < 0) spec_avg[i] = 0;
            if (spec_avg[i] > 127) spec_avg[i] = 127;
        }
        if (bk4829_afc_is_valid()) {
            /* Tripla coerenza soltanto con misura AFC realmente validata. */
            spur_update(&spur, spec_avg, afc_raw, spec_clean);
        } else {
            /* Non simulare AFC stabile con un valore costante zero. */
            for (int i = 0; i < SPEC_BINS; ++i)
                spec_clean[i] = (int8_t)spec_avg[i];
        }
        stabilizer_apply(&stab, spec_clean, spec_stable, SPEC_BINS);

        /* Live, Peak Hold e Waterfall restano percorsi distinti.
         * Il renderer live non applica cross-fade o clamp gradiente.
         */
        ui_spectrum_draw(spec_stable, SPEC_BINS);
        waterfall_push(spec_stable, SPEC_BINS);
        st7565_blit_full();
        /* Pausa minima tra frame: limita il refresh a circa 10-15 fps
         * con il clock core di riferimento 8 MHz configurato in hw_init. */
        delay_ms(67u);
        if (ui_spectrum_poll_event()) ui_spectrum_handle();
    }
}
