#include "system_init.h"
#include <stdint.h>

/* PY32F071 RCC and Cortex-M0+ SCB registers. */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define RCC_CR       REG32(0x40021000u)
#define RCC_ICSCR    REG32(0x40021004u)
#define RCC_CFGR     REG32(0x40021008u)
#define SCB_VTOR     REG32(0xE000ED08u)

#define RCC_CR_HSION       (1u << 8)
#define RCC_CR_HSIRDY      (1u << 10)
#define RCC_CR_HSIDIV_MASK (7u << 11)
#define RCC_CFGR_SW_MASK   (7u << 0)
#define RCC_CFGR_SWS_MASK  (7u << 3)
#define RCC_CFGR_HPRE_MASK (15u << 8)

/* Puya factory calibration locations, per system_py32f071.c (BSD-3-Clause). */
#define HSI_TRIM_FACTORY_0 REG32(0x1FFF3208u)
#define HSI_TRIM_FACTORY_1 REG32(0x1FFF3348u)

void SystemInit(void)
{
    /* Select HSI frequency code 1 = 8 MHz and restore factory trim.
     * This makes delay_us()/delay_ms() and the RF bit-bang timing deterministic
     * even if the DFU bootloader left a different clock configuration.
     */
    RCC_ICSCR = (RCC_ICSCR & 0xFE000000u) |
                (1u << 13) |
                (HSI_TRIM_FACTORY_0 & 0x0000FFFFu) |
                (HSI_TRIM_FACTORY_1 << 16);

    /* Enable HSI and force its divider to /1 (8 MHz). */
    RCC_CR = (RCC_CR & ~RCC_CR_HSIDIV_MASK) | RCC_CR_HSION;
    while ((RCC_CR & RCC_CR_HSIRDY) == 0u) { }

    /* Select HSI as SYSCLK and HCLK /1. Preserve unrelated CFGR fields. */
    RCC_CFGR &= ~(RCC_CFGR_SW_MASK | RCC_CFGR_HPRE_MASK);
    while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != 0u) { }

    /* The application vector table lives at the linker-defined K1 offset. */
    SCB_VTOR = 0x08002800u;
}
