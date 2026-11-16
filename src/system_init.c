#include <stdint.h>

/* Handoff minimale: non riconfigura clock e PLL.
 * Il bootloader K1 lascia lo stato clock in uso; prima della release occorre
 * misurare/identificare il clock effettivo e fissare HW_CORE_CLOCK_HZ.
 */
void SystemInit(void)
{
    /* TODO: clock tree verificato per il firmware target. */
}
