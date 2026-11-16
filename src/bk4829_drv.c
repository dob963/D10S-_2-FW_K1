#include "bk4829_drv.h"
#include "hw_init.h"

/* Driver adattato per BK4829 UV-K1 V3.
 *
 * Fonti K1 consultate (lettura, adattamento indipendente):
 * - App/apps/spectrum3d/spectrum3d_app.c, righe 110-116 e 232-241:
 *   REG_FREQ_LO=0x38, REG_FREQ_HI=0x39, REG_CTRL=0x30; frequenze
 *   espresse in unità da 10 Hz; sequenza di retune 38/39/30.
 * - App/apps/spectrum3d/spectrum3d_app.c, righe 255-265:
 *   il primo campione RSSI dopo il cambio frequenza può essere vecchio.
 * - App/driver/bk4829.c, righe 1550-1556:
 *   RSSI letto da REG 0x67, maschera 0x01FF; glitch da REG 0x63.
 * - App/driver/bk4829.c, righe 428-432:
 *   conversione upstream dBm = (RSSI / 2) - 160.
 *
 * Attenzione: il fork mantiene simboli BK4819_* e include bk4819.h.
 * Queste letture sono riferimenti specifici del firmware UV-K1 chiamato
 * bk4829.c, non una mappa registri BK4829 indipendente. Non importare
 * valori di init da BK4819 senza dump stock 7.03.01.
 */

#define BK_REG_FREQ_LO 0x38u
#define BK_REG_FREQ_HI 0x39u
#define BK_REG_CTRL    0x30u
#define BK_REG_RSSI    0x67u
#define BK_REG_GLITCH  0x63u
#define BK_CTRL_AF_DAC (1u << 9)

static uint16_t scan_ctrl;
static uint8_t scan_ctrl_valid;

void bk4829_init(void)
{
    /* Inizializzazione non distruttiva: conserva REG_30 letto dal chip
     * e mette in cache il profilo RX con AF DAC disabilitato.
     *
     * TODO BLOCCANTE: sequenza completa di reset/init RF da dump stock
     * UV-K1 bootloader 7.03.01. Il datasheet pubblico non contiene la
     * tabella dei registri di configurazione.
     */
    uint16_t raw_ctrl = bk_spi_read(BK_REG_CTRL);
    if (raw_ctrl != 0xFFFFu) {
        scan_ctrl = (uint16_t)(raw_ctrl & (uint16_t)~BK_CTRL_AF_DAC);
        scan_ctrl_valid = 1u;
    } else {
        scan_ctrl_valid = 0u; /* bus read non valido: non scrivere sul chip */
    }
}

void bk4829_set_freq(uint32_t freq_hz)
{
    /* Il driver Spectrum3D K1 usa frequenze in unità da 10 Hz:
     * 6.25 kHz = 625. Fonte: App/apps/spectrum3d/spectrum3d_app.c,
     * definizione CENTRE_STEP e funzione tune() (righe 84 e 232-241).
     */
    /* Limiti RX dichiarati dal datasheet BK4829 DS-BK4829-E01 V1.0:
     * 18-580 MHz e 760-1160 MHz. Fuori banda non inviare scritture PLL.
     */
    if (!((freq_hz >= 18000000u && freq_hz <= 580000000u) ||
          (freq_hz >= 760000000u && freq_hz <= 1160000000u))) return;

    uint32_t f10 = freq_hz / 10u;

    if (!scan_ctrl_valid) {
        uint16_t raw_ctrl = bk_spi_read(BK_REG_CTRL);
        if (raw_ctrl == 0xFFFFu) return;
        scan_ctrl = (uint16_t)(raw_ctrl & (uint16_t)~BK_CTRL_AF_DAC);
        scan_ctrl_valid = 1u;
    }

    /* Fonte K1 Spectrum3D: scrittura split 32 bit su 0x38/0x39,
     * poi reset/restore di REG_30 per riavviare la sintonia PLL.
     */
    bk_spi_write(BK_REG_FREQ_LO, (uint16_t)f10);
    bk_spi_write(BK_REG_FREQ_HI, (uint16_t)(f10 >> 16));
    bk_spi_write(BK_REG_CTRL, 0u);
    bk_spi_write(BK_REG_CTRL, scan_ctrl);
}

void bk4829_wait_settle(void)
{
    /* Fonte K1 Spectrum3D, App/apps/spectrum3d/spectrum3d_app.c:
     * attende REG_63 (low byte) < 200, massimo 50 letture, poi scarta
     * una lettura REG_67 perché può riferirsi alla frequenza precedente.
     * Il datasheet BK4829 richiede tNXT >= 1 us dopo l'indirizzo.
     */
    delay_ms(1u); /* margine conservativo rispetto al PLL <0,3 ms */
    uint8_t guard = 50u;
    while (guard-- && (bk_spi_read(BK_REG_GLITCH) & 0x00FFu) >= 200u)
        delay_us(1u);
    (void)bk_spi_read(BK_REG_RSSI);
}

int16_t bk4829_read_rssi(void)
{
    /* Fonte K1 App/driver/bk4829.c, righe 1550-1553:
     * return ReadRegister(REG_67) & 0x01FF.
     * Restituisce RSSI grezzo 9 bit, non dBm.
     */
    uint16_t raw = bk_spi_read(BK_REG_RSSI);
    if (raw == 0xFFFFu) return 0; /* bus non valido: non mostrare falso picco */
    return (int16_t)(raw & 0x01FFu);
}

int16_t bk4829_read_afc(void)
{
    /* TODO BLOCCANTE: candidato community non verificato:
     * fagci/k1-scaner, src/driver/bk4829.c righe 1330-1334 legge REG 0x6D
     * come int16_t e applica 5/6 Hz per unità. Il progetto dichiara
     * compatibilità BK4829/BK4819 e mantiene simboli BK4819_*: non è prova
     * indipendente della codifica sul BK4829 del nostro UV-K1 V3.
     * REG_73 è controllo/abilitazione AFC, non il valore misurato.
     * Verificare 0x6D, segno e scala sul dump stock 7.03.01 prima dell'uso.
     */
    return 0;
}
