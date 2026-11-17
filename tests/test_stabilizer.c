#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "stabilizer.h"

int main(void)
{
    stabilizer_t state;
    int8_t in[SPEC_BINS] = {0};
    int8_t out[SPEC_BINS] = {0};

    stabilizer_init(&state);

    /* Un segnale debole deve comparire: non può restare bloccato sotto
     * la soglia di salto STAB_HI.
     */
    in[10] = 5;
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 1);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 2);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 3);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 5);

    /* Un salto netto viene acquisito immediatamente. */
    in[10] = 40;
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 40);

    /* La discesa mantiene il picco per il tempo previsto e poi rilascia. */
    in[10] = 20;
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 40);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 40);
    stabilizer_apply(&state, in, out, SPEC_BINS);
    assert(out[10] == 38);

    puts("Stabilizer tests: PASS");
    return 0;
}
