#include "calib.h"
#include "spur_reject.h"
#include <stdint.h>

static int16_t dc_offset[SPEC_BINS];

/* TODO BLOCCANTE: indirizzo e formato pagina calibrazione non definiti.
 * Il toolkit UV-K1 identifica dati di fabbrica in area sensibile e il
 * bootloader/DFU occupa flash prima dell'applicazione. Non cancellare o
 * programmare alcuna pagina fino a una mappa completa e a un backup valido.
 */
void calib_load(void)
{
    /* Fallback sicuro: nessuna sottrazione DC senza record validato.
     * La lettura flash si attiva solo dopo aver definito settore dedicato,
     * formato, versione e CRC.
     */
    for (int i = 0; i < SPEC_BINS; ++i) dc_offset[i] = 0;
}

int16_t calib_apply_dc(int16_t v, int bin)
{
    if (bin < 0 || bin >= SPEC_BINS) return v;
    {
        int32_t out = (int32_t)v - dc_offset[bin];
        if (out < 0) out = 0;
        if (out > 32767) out = 32767;
        return (int16_t)out;
    }
}

int calib_save(const int16_t *offsets, uint16_t count)
{
    (void)offsets;
    (void)count;
    /* TODO BLOCCANTE: nessuna scrittura flash implementata intenzionalmente.
     * Una pagina scelta arbitrariamente potrebbe cancellare firmware o dati
     * stock. Restituisce -1 per indicare operazione non supportata.
     */
    return -1;
}
