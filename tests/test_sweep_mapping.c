#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "sweep_engine.h"

static int16_t fake_rssi;
static uint32_t tuned[ SPEC_BINS ];
static unsigned tune_count;

void bk4829_set_freq(uint32_t hz)
{
    if (tune_count < SPEC_BINS) tuned[tune_count] = hz;
    ++tune_count;
}
void bk4829_wait_settle(void) { }
int16_t bk4829_read_rssi(void) { return fake_rssi; }
int16_t bk4829_read_afc(void) { return 0; }

static unsigned count_nonzero(const int16_t *v)
{
    unsigned n = 0u;
    for (int i=0;i<SPEC_BINS;++i) if (v[i] != 0) ++n;
    return n;
}

static unsigned count_valid(const uint8_t *v)
{
    unsigned n = 0u;
    for (int i=0;i<SPEC_BINS;++i) if (v[i]) ++n;
    return n;
}

int main(void)
{
    int16_t spectrum[SPEC_BINS];
    int16_t afc[SPEC_BINS];
    uint8_t valid[SPEC_BINS];

    /* Sweep coarse: 32 misure su 128 bin, collocate sul vero asse frequenza. */
    fake_rssi = 50;
    tune_count = 0u;
    sweep_adaptive(144000000u, 148000000u, spectrum, afc, valid);
    assert(tune_count == 32u);
    assert(count_nonzero(spectrum) == 32u);
    assert(count_valid(valid) == 32u);
    for (int i=0;i<SPEC_BINS;++i) {
        if ((i % 4) == 0) { assert(spectrum[i] == 50); assert(valid[i] == 1u); }
        else { assert(spectrum[i] == 0); assert(valid[i] == 0u); }
    }
    assert(tuned[0] == 144000000u);
    assert(tuned[31] == 147875000u);

    /* Sweep fine: tutte le 128 frequenze occupano i rispettivi bin. */
    fake_rssi = 100;
    tune_count = 0u;
    sweep_adaptive(144000000u, 148000000u, spectrum, afc, valid);
    assert(tune_count == SPEC_BINS);
    assert(count_nonzero(spectrum) == SPEC_BINS);
    assert(count_valid(valid) == SPEC_BINS);
    for (int i=0;i<SPEC_BINS;++i) assert(spectrum[i] == 100);

    /* Span piccolo: step minimo 1 Hz evita loop infinito. */
    fake_rssi = 100;
    tune_count = 0u;
    sweep_adaptive(1000u, 1050u, spectrum, afc, valid);
    assert(tune_count == 50u);
    assert(count_nonzero(spectrum) == 50u);
    assert(count_valid(valid) == 50u);

    /* Range non valido: array azzerati, nessuna sintonia. */
    tune_count = 0u;
    sweep_adaptive(2000u, 1000u, spectrum, afc, valid);
    assert(tune_count == 0u);
    assert(count_nonzero(spectrum) == 0u);
    assert(count_valid(valid) == 0u);

    puts("Adaptive sweep frequency-bin mapping tests: PASS");
    return 0;
}
