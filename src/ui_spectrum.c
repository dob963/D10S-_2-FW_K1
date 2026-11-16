#include "ui_spectrum.h"

/* Peak Hold esclusivamente visuale: non modifica il buffer live. */
static int8_t peak[SPEC_BINS];

void ui_spectrum_init(void)
{
    for (int i = 0; i < SPEC_BINS; ++i) peak[i] = 0;
}

void ui_spectrum_draw(const int8_t *spec, int n)
{
    if (n > SPEC_BINS) n = SPEC_BINS;
    for (int i = 0; i < n; ++i) {
        int8_t live = spec[i] < 0 ? 0 : spec[i];

        /* Aggiorna solo il marker Peak Hold. Il valore live resta quello
         * ricevuto dallo stabilizer: nessun cross-fade, clamp o smoothing.
         */
        if (live > peak[i]) peak[i] = live;
        else if (peak[i] > 0) --peak[i];

        /* TODO: passare live e peak[i] al renderer ST7565.
         * Il renderer dovra' disegnare live direttamente, senza alterarlo.
         */
    }
}

int ui_spectrum_poll_event(void)
{
    return 0; /* TODO: collegare scansione matrice tastiera V3. */
}

void ui_spectrum_handle(void)
{
    /* TODO: gestione eventi UI dopo integrazione keypad. */
}
