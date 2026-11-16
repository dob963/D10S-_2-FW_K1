#ifndef CALIB_H
#define CALIB_H
#include <stdint.h>
void    calib_load(void);
int16_t calib_apply_dc(int16_t v, int bin);
/* Scrittura disabilitata finche' non e' riservata una pagina flash
 * senza sovrapporsi a bootloader, firmware o calibrazione stock. */
int calib_save(const int16_t *offsets, uint16_t count);
#endif
