#ifndef HW_INIT_H
#define HW_INIT_H
#include <stdint.h>

/* Inizializzazione minima PY32F071 e bus 3-wire BK4829. */
void hw_init(void);
void delay_ms(uint32_t ms);

/* Transazioni raw BK4829: indirizzo 7 bit, dato 16 bit.
 * Il formato R/W e' implementato secondo il frame 3-wire del datasheet;
 * verificare la polarita' R/W con dump/logic analyzer prima del collegamento RF. */
void bk_spi_write(uint8_t reg, uint16_t value);
uint16_t bk_spi_read(uint8_t reg);

#endif
