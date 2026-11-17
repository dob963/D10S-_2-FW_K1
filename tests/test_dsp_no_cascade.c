#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "median3.h"
#include "calib.h"
#include "stabilizer.h"
#include "waterfall.h"

static unsigned pixel_calls;

void st7565_set_pixel(int x, int y, int on)
{
    (void)x; (void)y; (void)on;
    ++pixel_calls;
}

int main(void)
{
    stabilizer_t state;
    int8_t input[SPEC_BINS] = {0};
    int8_t output[SPEC_BINS] = {0};

    assert(median3(1, 9, 5) == 5);
    assert(median3(-4, 2, 0) == 0);

    calib_load();
    assert(calib_apply_dc(123, 0) == 123);
    assert(calib_apply_dc(-2, 0) == 0);
    assert(calib_apply_dc(123, -1) == 123);
    assert(calib_apply_dc(123, SPEC_BINS) == 123);

    /* Un singolo bin alto non deve propagarsi ai bin adiacenti. */
    stabilizer_init(&state);
    input[64] = 100;
    stabilizer_apply(&state, input, output, SPEC_BINS);
    assert(output[64] == 100);
    for (int i = 0; i < SPEC_BINS; ++i)
        if (i != 64) assert(output[i] == 0);

    /* Hold/release resta sullo stesso bin: nessun cross-fade laterale. */
    memset(input, 0, sizeof(input));
    stabilizer_apply(&state, input, output, SPEC_BINS);
    for (int i = 0; i < SPEC_BINS; ++i)
        if (i != 64) assert(output[i] == 0);

    /* Waterfall OFF non deve scrivere pixel né interferire con lo stabilizer. */
    waterfall_clear();
    waterfall_set_enabled(0);
    pixel_calls = 0;
    waterfall_push(input, SPEC_BINS);
    assert(!waterfall_is_enabled());
    assert(pixel_calls == 0u);

    /* Riattivato, il Waterfall disegna soltanto la propria area 16x128. */
    waterfall_set_enabled(1);
    waterfall_push(input, SPEC_BINS);
    assert(waterfall_is_enabled());
    assert(pixel_calls == 16u * 128u);

    puts("DSP no-cascade and waterfall isolation tests: PASS");
    return 0;
}
