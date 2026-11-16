#include "hw_init.h"
#include "bk4829_drv.h"
#include <stdint.h>

/* PY32F071: indirizzi dal Reference Manual Puya PY32F07X.
 * GPIOA=0x50000000, GPIOB=0x50000400, GPIOF=0x50001400.
 * RCC=0x40021000; RCC_IOPENR offset 0x34, GPIOA/B/F enable bit 0/1/5.
 */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define RCC_IOPENR REG32(0x40021034u)
#define GPIOA_BASE 0x50000000u
#define GPIOB_BASE 0x50000400u
#define GPIOF_BASE 0x50001400u
#define GPIO_MODER(base) REG32((base) + 0x00u)
#define GPIO_OTYPER(base) REG32((base) + 0x04u)
#define GPIO_OSPEEDR(base) REG32((base) + 0x08u)
#define GPIO_PUPDR(base) REG32((base) + 0x0Cu)
#define GPIO_IDR(base) REG32((base) + 0x10u)
#define GPIO_BSRR(base) REG32((base) + 0x18u)

#define RF_SCK_PIN 8u       /* PB8: BK4829 SCK */
#define RF_SDIO_PIN 9u      /* PB9: BK4829 SDATA bidirezionale */
#define RF_SCN_PIN 9u       /* PF9: BK4829 SCN/CSN */

#define LCD_SCK_PIN 5u      /* PA5: ST7565 SCK */
#define LCD_A0_PIN 6u       /* PA6: ST7565 A0 */
#define LCD_MOSI_PIN 7u     /* PA7: ST7565 SDA */
#define LCD_CS_PIN 2u       /* PB2: ST7565 CS */

#ifndef HW_CORE_CLOCK_HZ
/* Valore prudenziale richiesto dal progetto; verificare il clock effettivo
 * lasciato dal bootloader prima di usare delay temporali come misura RF. */
#define HW_CORE_CLOCK_HZ 8000000u
#endif

#define SYST_CSR REG32(0xE000E010u)
#define SYST_RVR REG32(0xE000E014u)
#define SYST_CVR REG32(0xE000E018u)
#define SYST_CSR_ENABLE (1u << 0)
#define SYST_CSR_CLKSOURCE (1u << 2)
#define SYST_CSR_COUNTFLAG (1u << 16)

static void pin_mode(uint32_t base, uint32_t pin, uint32_t mode)
{
    uint32_t shift = pin * 2u;
    uint32_t v = GPIO_MODER(base);
    v &= ~(3u << shift);
    v |= ((mode & 3u) << shift);
    GPIO_MODER(base) = v;
}

static void pin_write(uint32_t base, uint32_t pin, uint32_t high)
{
    GPIO_BSRR(base) = high ? (1u << pin) : (1u << (pin + 16u));
}

static uint32_t pin_read(uint32_t base, uint32_t pin)
{
    return (GPIO_IDR(base) >> pin) & 1u;
}

static void rf_sdio_output(void)
{
    pin_mode(GPIOB_BASE, RF_SDIO_PIN, 1u);
    GPIO_OTYPER(GPIOB_BASE) &= ~(1u << RF_SDIO_PIN);
}

static void rf_sdio_input(void)
{
    pin_mode(GPIOB_BASE, RF_SDIO_PIN, 0u);
    GPIO_PUPDR(GPIOB_BASE) &= ~(3u << (RF_SDIO_PIN * 2u));
}

static void rf_send_bit(uint32_t bit)
{
    pin_write(GPIOB_BASE, RF_SDIO_PIN, bit);
    __asm volatile ("nop");
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    __asm volatile ("nop");
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
}

static uint32_t rf_read_bit(void)
{
    uint32_t bit;
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    __asm volatile ("nop");
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    __asm volatile ("nop");
    bit = pin_read(GPIOB_BASE, RF_SDIO_PIN);
    return bit;
}

void delay_ms(uint32_t ms)
{
    /* SysTick con clock core: nessun loop calibrato a numero di NOP.
     * Il clock va confermato sul target; timeout se reload non rappresentabile. */
    const uint32_t ticks_per_ms = HW_CORE_CLOCK_HZ / 1000u;
    if (ticks_per_ms == 0u || ticks_per_ms > 0x01000000u) return;
    SYST_CSR = 0u;
    SYST_RVR = ticks_per_ms - 1u;
    while (ms--) {
        SYST_CVR = 0u;
        SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_ENABLE;
        while ((SYST_CSR & SYST_CSR_COUNTFLAG) == 0u) { }
        SYST_CSR = 0u;
    }
}

void bk_spi_write(uint8_t reg, uint16_t value)
{
    int bit;
    reg &= 0x7Fu;
    pin_write(GPIOF_BASE, RF_SCN_PIN, 0u);
    rf_sdio_output();
    /* BK4829 3-wire: R/W + A6..A0 + D15..D0, MSB first.
     * Datasheet timing: latch on SCK rising edge. R/W=0 (write) da confermare
     * con cattura SPI sulla scheda UV-K1 V3 prima di autorizzare scritture RF. */
    rf_send_bit(0u);
    for (bit = 6; bit >= 0; --bit) rf_send_bit((reg >> bit) & 1u);
    for (bit = 15; bit >= 0; --bit) rf_send_bit((value >> bit) & 1u);
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
}

uint16_t bk_spi_read(uint8_t reg)
{
    int bit;
    uint16_t value = 0u;
    reg &= 0x7Fu;
    pin_write(GPIOF_BASE, RF_SCN_PIN, 0u);
    rf_sdio_output();
    /* R/W=1 (read); polarita' da confermare con cattura del bus stock. */
    rf_send_bit(1u);
    for (bit = 6; bit >= 0; --bit) rf_send_bit((reg >> bit) & 1u);
    rf_sdio_input();
    /* Turnaround del bus: il datasheet mostra il passaggio MCU->BK4829
     * dopo l'indirizzo; tNXT minimo 1 us. */
    delay_ms(1u); /* TODO sostituire con delay sub-microsecondo misurato */
    for (bit = 15; bit >= 0; --bit)
        value = (uint16_t)((value << 1) | rf_read_bit());
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
    return value;
}

void hw_init(void)
{
    /* Clock I/O GPIOA, GPIOB e GPIOF: RCC_IOPENR bit 0,1,5. */
    RCC_IOPENR |= (1u << 0) | (1u << 1) | (1u << 5);

    /* BK4829 su PB8/PB9/PF9: pinout UV-K1 V3 documentato. */
    pin_mode(GPIOB_BASE, RF_SCK_PIN, 1u);
    pin_mode(GPIOB_BASE, RF_SDIO_PIN, 1u);
    pin_mode(GPIOF_BASE, RF_SCN_PIN, 1u);
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    pin_write(GPIOB_BASE, RF_SDIO_PIN, 0u);
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);

    /* ST7565: PA5 SCK, PA7 MOSI, PA6 A0, PB2 CS. */
    pin_mode(GPIOA_BASE, LCD_SCK_PIN, 1u);
    pin_mode(GPIOA_BASE, LCD_MOSI_PIN, 1u);
    pin_mode(GPIOA_BASE, LCD_A0_PIN, 1u);
    pin_mode(GPIOB_BASE, LCD_CS_PIN, 1u);
    pin_write(GPIOA_BASE, LCD_SCK_PIN, 0u);
    pin_write(GPIOA_BASE, LCD_MOSI_PIN, 0u);
    pin_write(GPIOA_BASE, LCD_A0_PIN, 0u);
    pin_write(GPIOB_BASE, LCD_CS_PIN, 1u);

    /* Keypad matrix PB3..PB6 / PB12..PB15: predisposizione input
     * con pull-up; la scansione e la polarita' restano nel driver keypad. */
    for (uint32_t p = 3u; p <= 6u; ++p) {
        pin_mode(GPIOB_BASE, p, 0u);
        GPIO_PUPDR(GPIOB_BASE) =
            (GPIO_PUPDR(GPIOB_BASE) & ~(3u << (p * 2u))) | (1u << (p * 2u));
    }
    for (uint32_t p = 12u; p <= 15u; ++p) {
        pin_mode(GPIOB_BASE, p, 0u);
        GPIO_PUPDR(GPIOB_BASE) =
            (GPIO_PUPDR(GPIOB_BASE) & ~(3u << (p * 2u))) | (1u << (p * 2u));
    }

    bk4829_init();
}
