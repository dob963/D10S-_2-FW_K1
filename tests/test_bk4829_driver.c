#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bk4829_drv.h"

static uint16_t regs[128];
static uint8_t write_regs[32];
static uint16_t write_values[32];
static unsigned write_count;
static unsigned glitch_index;
static unsigned glitch_count;
static uint16_t glitch_values[8];
static unsigned glitch_reads;
static unsigned rssi_reads;
static uint32_t ms_total;
static uint32_t us_total;

uint16_t bk_spi_read(uint8_t reg)
{
    if (reg == 0x63u) {
        ++glitch_reads;
        if (glitch_index < glitch_count) return glitch_values[glitch_index++];
        return regs[reg];
    }
    if (reg == 0x67u) ++rssi_reads;
    return regs[reg];
}

void bk_spi_write(uint8_t reg, uint16_t value)
{
    assert(write_count < 32u);
    write_regs[write_count] = reg;
    write_values[write_count] = value;
    ++write_count;
    regs[reg] = value;
}

void delay_ms(uint32_t ms) { ms_total += ms; }
void delay_us(uint32_t us) { us_total += us; }

static void reset_logs(void)
{
    write_count = 0u;
    glitch_index = 0u;
    glitch_count = 0u;
    glitch_reads = 0u;
    rssi_reads = 0u;
    ms_total = 0u;
    us_total = 0u;
    memset(write_regs, 0, sizeof(write_regs));
    memset(write_values, 0, sizeof(write_values));
}

int main(void)
{
    /* REG_30 letto dalla radio: il bit AF DAC (9) deve essere azzerato. */
    reset_logs();
    regs[0x30] = 0xA345u;
    bk4829_init();
    bk4829_set_freq(146000000u);

    const uint32_t f10 = 14600000u;
    assert(write_count == 4u);
    assert(write_regs[0] == 0x38u && write_values[0] == (uint16_t)f10);
    assert(write_regs[1] == 0x39u && write_values[1] == (uint16_t)(f10 >> 16));
    assert(write_regs[2] == 0x30u && write_values[2] == 0u);
    assert(write_regs[3] == 0x30u && write_values[3] == 0xA145u);

    /* RSSI grezzo: 9 bit; una lettura bus tutta alta è rifiutata. */
    regs[0x67] = 0x01ABu;
    assert(bk4829_read_rssi() == 0x01AB);
    regs[0x67] = 0xFFFFu;
    assert(bk4829_read_rssi() == 0);

    /* PLL: 1 ms conservativo, glitch < 200, poi scarto di un RSSI. */
    reset_logs();
    glitch_count = 3u;
    glitch_values[0] = 250u;
    glitch_values[1] = 220u;
    glitch_values[2] = 199u;
    regs[0x67] = 0x0055u;
    bk4829_wait_settle();
    assert(ms_total == 1u);
    assert(us_total == 2u);
    assert(glitch_reads == 3u);
    assert(rssi_reads == 1u);

    /* Se REG_30 non è leggibile (bus alto), nessuna scrittura di sintonia. */
    reset_logs();
    regs[0x30] = 0xFFFFu;
    bk4829_init();
    bk4829_set_freq(146000000u);
    bk4829_wait_settle();
    assert(bk4829_read_rssi() == 0);
    assert(write_count == 0u);
    assert(glitch_reads == 0u);
    assert(rssi_reads == 0u);

    /* Frequenze fuori dalle bande dichiarate dal datasheet: nessuna scrittura. */
    reset_logs();
    regs[0x30] = 0x1234u;
    bk4829_init();
    bk4829_set_freq(650000000u);
    assert(write_count == 0u);

    /* AFC resta esplicitamente non implementato. */
    assert(bk4829_read_afc() == 0);
    assert(bk4829_afc_is_valid() == 0);

    puts("BK4829 driver host tests: PASS");
    return 0;
}
