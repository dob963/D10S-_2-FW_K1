#include "agc_sw.h"
#include "spur_reject.h"
#include "sweep_engine.h"
#include "median3.h"
#include "waterfall.h"
#include "ui_spectrum.h"
#include "stabilizer.h"
#include "hw_init.h"
#include "calib.h"

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

    /* TODO: rendere start/stop configurabili dalla UI. */
    const uint32_t f_start = 144000000u;
    const uint32_t f_stop  = 148000000u;

    for (int i = 0; i < SPEC_BINS; ++i) spec_avg[i] = 0;

    for (;;) {
        sweep_adaptive(f_start, f_stop, spec_raw, afc_raw);
        sweep_average(spec_raw, spec_avg, SPEC_BINS, 128);

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

        /* NOTA: spur_update() ha output int8_t ma ingresso fino a 255.
         * La conversione interna attuale puo' andare in overflow oltre 127.
         * Non viene alterato qui il modulo DSP fornito: serve concordare
         * una rappresentazione comune (range 0..127 oppure API piu' ampia).
         * Fino ad allora questo percorso non e' certificato per ampiezza piena.
         */
        spur_update(&spur, spec_avg, afc_raw, spec_clean);
        stabilizer_apply(&stab, spec_clean, spec_stable, SPEC_BINS);

        /* Live, Peak Hold e Waterfall restano percorsi distinti.
         * Il renderer live non applica cross-fade o clamp gradiente.
         */
        ui_spectrum_draw(spec_stable, SPEC_BINS);
        waterfall_push(spec_stable, SPEC_BINS);
        if (ui_spectrum_poll_event()) ui_spectrum_handle();
    }
}
