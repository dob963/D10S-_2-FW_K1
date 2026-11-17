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
/* BK4829 GPIO outputs: GPIO3=UHF LNA, GPIO4=VHF LNA.
 * Bit map follows REG_33 GPIO0..GPIO6 encoding (0x40 >> pin).
 */
static uint16_t rf_gpio_state = 0x9000u;

void bk4829_init(void)
{
    /* Sequenza di init pubblicata dal fork F4HWN per UV-K1:
     * App/driver/bk4829.c, funzione BK4819_Init(), commit
     * 3261ca7883d5b0b60c0905d8766612dee7b4191d, righe 100-201.
     *
     * Il nome BK4819_* è legacy: il file fisico e i pin appartengono
     * al target V3/K1. Non si importa la sequenza da bk4819.c V1/V2.
     * La tabella qui sotto è una reimplementazione indipendente dei
     * valori esposti dal driver UV-K1; fonte/licenza registrate nei docs.
     */
    static const struct { uint8_t reg; uint16_t value; } init_seq[] = {
        {0x00u, 0x8000u}, {0x00u, 0x0000u},
        {0x37u, 0x9D1Fu}, {0x36u, 0x0022u},
        {0x10u, 0x0318u}, {0x11u, 0x033Au},
        {0x12u, 0x03DBu}, {0x13u, 0x03DFu},
        {0x14u, 0x0210u}, {0x49u, 0x2AB2u},
        {0x7Bu, 0x73DCu}, {0x7Du, 0xE920u},
        {0x48u, 0x33A8u}, {0x40u, 0x3516u},
        {0x1Cu, 0x07C0u}, {0x1Du, 0xE555u},
        {0x1Eu, 0x4C58u}, {0x1Fu, 0xC65Au},
        {0x3Eu, 0x94C6u}, {0x73u, 0x4691u},
        {0x77u, 0x88EFu}, {0x19u, 0x1041u},
        {0x28u, 0x0B40u}, {0x29u, 0xAA00u},
        {0x2Au, 0x6600u}, {0x2Cu, 0x1822u},
        {0x2Fu, 0x9890u}, {0x53u, 0x2028u},
        {0x7Eu, 0x303Eu}, {0x46u, 0x600Au},
        {0x4Au, 0x5430u}, {0x07u, 0x61CEu},
        {0x33u, 0x9000u}, {0x3Fu, 0x0000u}
    };
    static const uint8_t dtmf_coeffs[16] = {
        111u, 107u, 103u, 98u, 80u, 71u, 58u, 44u,
        65u, 55u, 37u, 23u, 228u, 203u, 181u, 159u
    };

    for (unsigned i = 0u; i < sizeof(init_seq) / sizeof(init_seq[0]); ++i)
        bk_spi_write(init_seq[i].reg, init_seq[i].value);

    /* Tabella DTMF, stessa funzione/ordine del driver F4HWN bk4829.c. */
    for (unsigned i = 0u; i < sizeof(dtmf_coeffs); ++i)
        bk_spi_write(0x09u, (uint16_t)((i << 12) | dtmf_coeffs[i]));

    /* Stato iniziale GPIO del BK4829 come nel driver UV-K1: i bit
     * 0x08 (GPIO3/UHF) e 0x04 (GPIO4/VHF) partono entrambi spenti.
     */
    rf_gpio_state = 0x9000u;
    bk_spi_write(0x33u, rf_gpio_state);

    /* REG_30 è configurato a runtime dal firmware UV-K1. Non inventare
     * un valore iniziale: leggere quello presente dopo la sequenza init. */
    uint16_t raw_ctrl = bk_spi_read(BK_REG_CTRL);
    if (raw_ctrl != 0xFFFFu) {
        scan_ctrl = (uint16_t)(raw_ctrl & (uint16_t)~BK_CTRL_AF_DAC);
        scan_ctrl_valid = 1u;
    } else {
        scan_ctrl_valid = 0u;
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

    /* Selezione del ramo RF esterno: GPIO4 abilita VHF LNA (<280 MHz),
     * GPIO3 abilita UHF LNA (>=280 MHz). Il BK4829 usa REG_33 con
     * maschera 0x40>>GPIO_number; preserviamo tutti gli altri bit.
     * Riferimento comportamentale: driver UV-K1 bk4829.c, funzione
     * BK4819_PickRXFilterPathBasedOnFrequency (licenza Apache-2.0).
     */
    const uint16_t lna_mask = 0x000Cu; /* GPIO3=0x08, GPIO4=0x04 */
    const uint16_t lna_value = (freq_hz < 280000000u) ? 0x0004u : 0x0008u;
    const uint16_t next_gpio = (uint16_t)((rf_gpio_state & ~lna_mask) | lna_value);
    if (next_gpio != rf_gpio_state) {
        bk_spi_write(0x33u, next_gpio);
        rf_gpio_state = next_gpio;
    }

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
    if (!scan_ctrl_valid) return;
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
    if (!scan_ctrl_valid) return 0;
    /* Fonte K1 App/driver/bk4829.c, righe 1550-1553:
     * return ReadRegister(REG_67) & 0x01FF.
     * Restituisce RSSI grezzo 9 bit, non dBm.
     */
    uint16_t raw = bk_spi_read(BK_REG_RSSI);
    if (raw == 0xFFFFu) return 0; /* bus non valido: non mostrare falso picco */
    return (int16_t)(raw & 0x01FFu);
}

int bk4829_set_afc_enabled(int enabled)
{
    /* Fonte: F4HWN App/driver/bk4819-regs.h, afcDisableRegSpec
     * (REG 0x73, bit 4, maschera 0x0010); usato dal target UV-K1
     * in App/radio.c e App/driver/bk4829.c.
     * Bit=1 disabilita AFC, bit=0 abilita AFC. RMW preserva gli altri bit.
     */
    uint16_t value = bk_spi_read(0x73u);
    if (value == 0xFFFFu) return 0;
    if (enabled)
        value = (uint16_t)(value & (uint16_t)~0x0010u);
    else
        value = (uint16_t)(value | 0x0010u);
    bk_spi_write(0x73u, value);
    uint16_t verify = bk_spi_read(0x73u);
    if (verify == 0xFFFFu) return 0;
    return ((verify & 0x0010u) == (value & 0x0010u)) ? 1 : 0;
}

int16_t bk4829_read_afc(void)
{
    /* TODO: misura/offset AFC non deducibile dalla sola abilitazione REG 0x73.
     * AFCR 0-7 è un indizio community (4PDA), ma nel codice F4HWN
     * consultato non è stata trovata una lettura BK4829 con scala/segno
     * confermati. Cercare nel dump stock 7.03.01 e in una documentazione
     * BK4829 primaria; non usare REG 0x6D senza conferma indipendente.
     */
    return 0;
}


int bk4829_afc_is_valid(void)
{
    /* TODO: abilitare solo dopo conferma REG 0x6D e scala sul dump stock. */
    return 0;
}
