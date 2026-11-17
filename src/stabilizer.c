#include "stabilizer.h"
#include <string.h>

void stabilizer_init(stabilizer_t *s)
{
    if (!s) return;
    memset(s, 0, sizeof(*s));
}

void stabilizer_apply(stabilizer_t *s, const int8_t *in,
                      int8_t *out, int n)
{
    if (!s || !in || !out || n <= 0) return;
    if (n > SPEC_BINS) n = SPEC_BINS;

    for (int i = 0; i < n; ++i) {
        int target = in[i] < 0 ? 0 : in[i];
        int shown = s->shown[i] < 0 ? 0 : s->shown[i];
        int diff = target - shown;

        if (diff > 0) {
            /* Non bloccare segnali deboli o crescite lente sotto STAB_HI.
             * I salti grandi vengono acquisiti subito; i piccoli avanzano
             * di un livello per sweep, così la traccia non resta congelata.
             */
            s->shown[i] = (int8_t)(diff >= STAB_HI ? target : shown + 1);
            s->hold[i] = STAB_HOLD;
        } else if (diff < 0) {
            if (s->hold[i] > 0) {
                s->hold[i]--;
            } else if (-diff >= STAB_LO) {
                int next = shown - STAB_RELEASE;
                if (next < target) next = target;
                if (next < 0) next = 0;
                s->shown[i] = (int8_t)next;
            }
        }

        out[i] = s->shown[i];
    }
}
