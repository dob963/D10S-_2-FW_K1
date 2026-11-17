#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "sweep_engine.h"

static int16_t fake_rssi;
static uint32_t tuned[SPEC_BINS];
static unsigned tune_count;

void bk4829_set_freq(uint32_t hz)
{
    if (tune_count < SPEC_BINS) tuned[tune_count] = hz;
    ++tune_count;
}
void bk4829_wait_settle(void) { }
int16_t bk4829_read_rssi(void) { return fake_rssi; }
int16_t bk4829_read_afc(void) { return 0; }

static unsigned count_valid(const uint8_t *v)
{
    unsigned n = 0u;
    for (int i = 0; i < SPEC_BINS; ++i) if (v[i]) ++n;
    return n;
}

int main(void)
{
    int16_t spectrum[SPEC_BINS];
    int16_t afc[SPEC_BINS];
    uint8_t valid[SPEC_BINS];

    /* Anche con segnale debole: copertura completa e uniforme. */
    fake_rssi = 50;
    tune_count = 0u;
    sweep_adaptive(144000000u, 148000000u, spectrum, afc, valid);
    assert(tune_count == SPEC_BINS);
    assert(count_valid(valid) == SPEC_BINS);
    assert(tuned[0] == 144000000u);
    assert(tuned[SPEC_BINS - 1] == 148000000u);
    for (int i = 0; i < SPEC_BINS; ++i) {
        assert(spectrum[i] == 50);
        assert(valid[i] == 1u);
        assert(afc[i] == 0);
        if (i > 0) assert(tuned[i] > tuned[i - 1]);
    }

    /* Anche con segnale forte il numero di campioni non cambia. */
    fake_rssi = 200;
    tune_count = 0u;
    sweep_adaptive(144000000u, 148000000u, spectrum, afc, valid);
    assert(tune_count == SPEC_BINS);
    assert(count_valid(valid) == SPEC_BINS);

    /* Banda stretta: tutti i bin sono visitati; le frequenze possono
     * ripetersi se la banda ha meno Hz che bin disponibili.
     */
    tune_count = 0u;
    sweep_adaptive(1000u, 1050u, spectrum, afc, valid);
    assert(tune_count == SPEC_BINS);
    assert(count_valid(valid) == SPEC_BINS);
    assert(tuned[0] == 1000u);
    assert(tuned[SPEC_BINS - 1] == 1050u);
    for (int i = 1; i < SPEC_BINS; ++i)
        assert(tuned[i] >= tuned[i - 1]);

    /* Range non valido: array azzerati, nessuna sintonia. */
    tune_count = 0u;
    sweep_adaptive(2000u, 1000u, spectrum, afc, valid);
    assert(tune_count == 0u);
    assert(count_valid(valid) == 0u);
    for (int i = 0; i < SPEC_BINS; ++i) {
        assert(spectrum[i] == 0);
        assert(afc[i] == 0);
    }

    puts("Uniform complete sweep mapping tests: PASS");
    return 0;
}
