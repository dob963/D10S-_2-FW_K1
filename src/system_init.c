#include "system_init.h"

/* Handoff minimale: non riconfigura clock e non scrive registri SCB non
 * ancora confermati per la specifica implementazione PY32F071.
 *
 * Il bootloader K1 deve trasferire il controllo con il contesto vettori
 * corretto per l'applicazione a 0x08002800. Prima di abilitare IRQ occorre
 * verificare il supporto VTOR effettivo e il comportamento del bootloader.
 */
void SystemInit(void)
{
    /* TODO: clock e vector handoff verificati sul target. */
}
