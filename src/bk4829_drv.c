#include "bk4829_drv.h"
#include "hw_init.h"

/* Datasheet BK4829 DS-BK4829-E01 V1.0:
 * - interfaccia 3-wire: R/W + A[6:0] + D[15:0], MSB first;
 * - frequenza SCK massima 8 MHz;
 * - crystal nominale 26 MHz;
 * - PLL settle <0.3 ms.
 *
 * ATTENZIONE: il datasheet pubblico consultato non espone una tabella
 * completa dei registri di controllo, ne' codifica RSSI/AFC, ne' la formula
 * di programmazione frac-N. Queste operazioni sono TODO bloccanti.
 * Non inserire valori presi da BK4819/UV-K5.
 */

void bk4829_init(void)
{
    /* TODO BLOCCANTE:
     * sequenza reset e inizializzazione RX ricavata da dump stock UV-K1 V3.
     * Non effettuare scritture arbitrarie: rischio di mettere il chip in TX
     * o alterare calibrazioni/configurazioni RF.
     */
}

void bk4829_set_freq(uint32_t freq_hz)
{
    (void)freq_hz;
    /* TODO BLOCCANTE:
     * formula frac-N, Ndiv e fIF da documentazione di programmazione BK4829
     * o dump stock verificato. Non assumere un PLL formula BK4819.
     */
}

void bk4829_wait_settle(void)
{
    /* Datasheet: PLL settle <0.3 ms; 1 ms e' margine conservativo.
     * Il timing effettivo dipende dal clock core configurato. */
    delay_ms(1u);
}

int16_t bk4829_read_rssi(void)
{
    /* TODO BLOCCANTE: indirizzo registro, bitfield, segno e scala RSSI
     * devono essere identificati su datasheet esteso o dump stock.
     */
    return 0;
}

int16_t bk4829_read_afc(void)
{
    /* TODO BLOCCANTE: indirizzo registro e codifica AFC non presenti
     * nel datasheet pubblico DS-BK4829-E01 V1.0 consultato.
     */
    return 0;
}
