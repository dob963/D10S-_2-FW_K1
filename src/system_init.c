#include "system_init.h"
#include <stdint.h>

/* PY32F071 clock tree: decode the clock left by the UV-K1 bootloader.
 * Do not force HSI to 8 MHz: the K1 firmware environment may hand off with
 * another valid HCLK (including PLL at 48 MHz). Values/field encodings follow
 * the Puya PY32F071 CMSIS system source and reference header.
 */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define RCC_ICSCR    REG32(0x40021004u)
#define RCC_CFGR     REG32(0x40021008u)
#define RCC_CR       REG32(0x40021000u)
#define RCC_PLLCFGR  REG32(0x4002100Cu)
#define SCB_VTOR     REG32(0xE000ED08u)

#define RCC_CFGR_SWS_MASK   (7u << 3)
#define RCC_CFGR_HPRE_MASK  (15u << 8)
#define RCC_PLLCFGR_PLLSRC_MASK 3u
#define RCC_PLLCFGR_PLLMUL_MASK (3u << 2)
#define RCC_ICSCR_HSI_FS_MASK (7u << 13)
#define RCC_CR_HSIDIV_MASK (7u << 11)

#define HSE_VALUE_HZ 24000000u
#define LSI_VALUE_HZ 32768u
#define LSE_VALUE_HZ 32768u

uint32_t SystemCoreClock = 8000000u;

void SystemCoreClockUpdate(void)
{
    static const uint32_t hsi_freq_hz[8] = {
        4000000u, 8000000u, 16000000u, 22120000u,
        24000000u, 4000000u, 4000000u, 4000000u
    };
    /* Puya PLL multiplier encoding: 2, 3, 2, 2. */
    static const uint8_t pll_mul[4] = {2u, 3u, 2u, 2u};
    /* AHB prescaler encodings expressed as right shifts. */
    static const uint8_t ahb_shift[16] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        1u, 2u, 3u, 4u, 6u, 7u, 8u, 9u
    };

    const uint32_t cfgr = RCC_CFGR;
    const uint32_t sws = cfgr & RCC_CFGR_SWS_MASK;
    uint32_t sysclk = 0u;

    switch (sws) {
    case 0x00000000u: { /* HSI */
        const uint32_t fs = (RCC_ICSCR & RCC_ICSCR_HSI_FS_MASK) >> 13;
        const uint32_t div_code = (RCC_CR & RCC_CR_HSIDIV_MASK) >> 11;
        sysclk = hsi_freq_hz[fs] / (1u << div_code);
        break;
    }
    case 0x00000008u: /* HSE */
        sysclk = HSE_VALUE_HZ;
        break;
    case 0x00000010u: { /* PLL */
        const uint32_t pllcfgr = RCC_PLLCFGR;
        const uint32_t src = pllcfgr & RCC_PLLCFGR_PLLSRC_MASK;
        const uint32_t mul = (pllcfgr & RCC_PLLCFGR_PLLMUL_MASK) >> 2;
        if (src == 0x2u) {
            const uint32_t fs = (RCC_ICSCR & RCC_ICSCR_HSI_FS_MASK) >> 13;
            sysclk = hsi_freq_hz[fs] * pll_mul[mul];
        } else if (src == 0x3u) {
            sysclk = HSE_VALUE_HZ * pll_mul[mul];
        }
        break;
    }
    case 0x00000018u: /* LSI */
        sysclk = LSI_VALUE_HZ;
        break;
    case 0x00000020u: /* LSE */
        sysclk = LSE_VALUE_HZ;
        break;
    default:
        sysclk = 0u;
        break;
    }

    const uint32_t hpre = (cfgr & RCC_CFGR_HPRE_MASK) >> 8;
    SystemCoreClock = sysclk >> ahb_shift[hpre];
}

void SystemInit(void)
{
    /* PY32F071 K1 startup reference relocates vectors to FLASH + 0x2800.
     * Preserve the clock tree established by the bootloader and measure HCLK
     * from RCC registers instead of assuming a fixed 8 MHz or 48 MHz.
     */
    SCB_VTOR = 0x08002800u;
    SystemCoreClockUpdate();
}
