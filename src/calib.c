#include "calib.h"
#include "spur_reject.h"

static int16_t dc_offset[SPEC_BINS];

void calib_load(void)
{
    /* Default sicuro: nessun offset finché non esiste una calibrazione valida.
     *
     * TODO BLOCCANTE: leggere il dump stock 7.03.01 e definire una pagina
     * flash dedicata con magic/versione/CRC. Non usare gli indirizzi stock
     * 0x08001E00..0x08002000 (area calibrazione radio) e non scegliere una
     * pagina libera senza verificare erase size, bootloader e linker.
     */
    for (int i = 0; i < SPEC_BINS; ++i) dc_offset[i] = 0;
}

int16_t calib_apply_dc(int16_t v, int bin)
{
    if (bin < 0 || bin >= SPEC_BINS) return v;
    int16_t out = (int16_t)(v - dc_offset[bin]);
    if (out < 0) out = 0;
    return out;
}
