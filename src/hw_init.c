#include "hw_init.h"
#include "bk4829_drv.h"
#include "st7565.h"
#include <stdint.h>

/* PY32F071 register map. */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define RCC_IOPENR REG32(0x40021034u)
#define GPIOA_BASE 0x50000000u
#define GPIOB_BASE 0x50000400u
#define GPIOF_BASE 0x50001400u
#define GPIO_MODER(base) REG32((base) + 0x00u)
#define GPIO_OTYPER(base) REG32((base) + 0x04u)
#define GPIO_PUPDR(base) REG32((base) + 0x0Cu)
#define GPIO_IDR(base) REG32((base) + 0x10u)
#define GPIO_BSRR(base) REG32((base) + 0x18u)

#define RF_SCK_PIN 8u
#define RF_SDIO_PIN 9u
#define RF_SCN_PIN 9u
#define LCD_SCK_PIN 5u
#define LCD_A0_PIN 6u
#define LCD_MOSI_PIN 7u
#define LCD_CS_PIN 2u

extern uint32_t SystemCoreClock;
extern void SystemCoreClockUpdate(void);
#define SYST_CSR REG32(0xE000E010u)
#define SYST_RVR REG32(0xE000E014u)
#define SYST_CVR REG32(0xE000E018u)
#define SYST_CSR_ENABLE (1u << 0)
#define SYST_CSR_CLKSOURCE (1u << 2)
#define SYST_CSR_COUNTFLAG (1u << 16)

/* Ritardo locale con barriera: 6 iterazioni NOP più overhead di loop
 * (circa 35-45 cicli), senza importare la macro assembly upstream.
 * Frequenza core fissata a 8 MHz da SystemInit.
 */
static void rf_short_delay(void)
{
    for (volatile uint32_t i = 0u; i < 6u; ++i)
        __asm volatile ("nop" ::: "memory");
}

static void pin_mode(uint32_t base, uint32_t pin, uint32_t mode)
{
    uint32_t shift = pin * 2u;
    uint32_t v = GPIO_MODER(base);
    v = (v & ~(3u << shift)) | ((mode & 3u) << shift);
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
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    rf_short_delay();
}

static uint32_t rf_read_bit(void)
{
    uint32_t bit;
    /* Il BK4829 campiona SDA nella fase SCL bassa, prima del fronte
     * di salita; leggere dopo il fronte di discesa sposta il bit.
     * Sequenza allineata al driver UV-K1 upstream ReadU16(). */
    bit = pin_read(GPIOB_BASE, RF_SDIO_PIN);
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    rf_short_delay();
    return bit;
}

void delay_us(uint32_t us)
{
    const uint32_t ticks_per_us = SystemCoreClock / 1000000u;
    if (ticks_per_us == 0u || ticks_per_us > 0x01000000u) return;
    SYST_CSR = 0u;
    SYST_RVR = ticks_per_us - 1u;
    while (us--) {
        SYST_CVR = 0u;
        SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_ENABLE;
        while ((SYST_CSR & SYST_CSR_COUNTFLAG) == 0u) { }
        SYST_CSR = 0u;
    }
}

void delay_ms(uint32_t ms)
{
    const uint32_t ticks_per_ms = SystemCoreClock / 1000u;
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
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    rf_short_delay();
    pin_write(GPIOF_BASE, RF_SCN_PIN, 0u);
    rf_sdio_output();
    rf_send_bit(0u);
    for (bit = 6; bit >= 0; --bit) rf_send_bit((reg >> bit) & 1u);
    for (bit = 15; bit >= 0; --bit) rf_send_bit((value >> bit) & 1u);
    rf_short_delay();
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    pin_write(GPIOB_BASE, RF_SDIO_PIN, 1u);
}

uint16_t bk_spi_read(uint8_t reg)
{
    int bit;
    uint16_t value = 0u;
    reg &= 0x7Fu;
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    rf_short_delay();
    pin_write(GPIOF_BASE, RF_SCN_PIN, 0u);
    rf_sdio_output();
    rf_send_bit(1u);
    for (bit = 6; bit >= 0; --bit) rf_send_bit((reg >> bit) & 1u);
    rf_sdio_input();
    rf_short_delay();
    for (bit = 15; bit >= 0; --bit)
        value = (uint16_t)((value << 1) | rf_read_bit());
    rf_sdio_output();
    rf_short_delay();
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);
    rf_short_delay();
    pin_write(GPIOB_BASE, RF_SCK_PIN, 1u);
    pin_write(GPIOB_BASE, RF_SDIO_PIN, 1u);
    return value;
}

/* UV-K1 keypad: four active-low scan columns PB6..PB3 and rows PB15..PB12.
 * Side keys are sensed with all columns high (the upstream matrix's column 0).
 */
static void keypad_drive_columns(uint8_t low_column)
{
    static const uint8_t pins[4] = {6u, 5u, 4u, 3u};
    for (unsigned i = 0; i < 4u; ++i) pin_write(GPIOB_BASE, pins[i], 1u);
    if (low_column < 4u) pin_write(GPIOB_BASE, pins[low_column], 0u);
}

void hw_init(void)
{
    /* Reset_Handler calls SystemInit before .data copy; refresh HCLK only
     * now, after C runtime initialization, so the global is not overwritten. */
    SystemCoreClockUpdate();
    RCC_IOPENR |= (1u << 0) | (1u << 1) | (1u << 5);

    pin_mode(GPIOB_BASE, RF_SCK_PIN, 1u);
    pin_mode(GPIOB_BASE, RF_SDIO_PIN, 1u);
    pin_mode(GPIOF_BASE, RF_SCN_PIN, 1u);
    pin_mode(GPIOF_BASE, 8u, 1u); /* PF8 backlight, active high */
    pin_write(GPIOF_BASE, 8u, 0u); /* keep dark during controller init */
    pin_write(GPIOB_BASE, RF_SCK_PIN, 0u);
    pin_write(GPIOB_BASE, RF_SDIO_PIN, 0u);
    pin_write(GPIOF_BASE, RF_SCN_PIN, 1u);

    pin_mode(GPIOA_BASE, LCD_SCK_PIN, 1u);
    pin_mode(GPIOA_BASE, LCD_MOSI_PIN, 1u);
    pin_mode(GPIOA_BASE, LCD_A0_PIN, 1u);
    pin_mode(GPIOB_BASE, LCD_CS_PIN, 1u);
    pin_write(GPIOA_BASE, LCD_SCK_PIN, 0u);
    pin_write(GPIOA_BASE, LCD_MOSI_PIN, 0u);
    pin_write(GPIOA_BASE, LCD_A0_PIN, 0u);
    pin_write(GPIOB_BASE, LCD_CS_PIN, 1u);

    /* Correct matrix direction: columns are driven outputs, rows are
     * pulled-up inputs. PTT PB10 is also an active-low pulled-up input. */
    for (uint32_t p = 3u; p <= 6u; ++p) {
        pin_mode(GPIOB_BASE, p, 1u);
        GPIO_PUPDR(GPIOB_BASE) &= ~(3u << (p * 2u));
    }
    for (uint32_t p = 12u; p <= 15u; ++p) {
        pin_mode(GPIOB_BASE, p, 0u);
        GPIO_PUPDR(GPIOB_BASE) =
            (GPIO_PUPDR(GPIOB_BASE) & ~(3u << (p * 2u))) | (1u << (p * 2u));
    }
    pin_mode(GPIOB_BASE, 10u, 0u);
    GPIO_PUPDR(GPIOB_BASE) =
        (GPIO_PUPDR(GPIOB_BASE) & ~(3u << 20u)) | (1u << 20u);
    keypad_drive_columns(0xFFu);

    bk4829_init();
    st7565_init();
    pin_write(GPIOF_BASE, 8u, 1u); /* full backlight after ST7565 is ready */
}
