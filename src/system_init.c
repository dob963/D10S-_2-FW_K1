#include "system_init.h"
#include <stdint.h>

/* PY32F071 / Cortex-M0+: registro VTOR dello SCB.
 * L'immagine applicativa è linkata a 0x08002800 (vedi linker script).
 * Il bootloader può trasferire il controllo senza lasciare VTOR aggiornato:
 * riallineiamo quindi la tabella vettori prima di inizializzare .data/.bss
 * e prima che il firmware abiliti eventuali interrupt.
 */
#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08u)
#define APP_VECTOR_BASE 0x08002800u

void SystemInit(void)
{
    SCB_VTOR = APP_VECTOR_BASE;
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}
