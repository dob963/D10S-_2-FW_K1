#ifndef UI_SPECTRUM_H
#define UI_SPECTRUM_H
#include <stdint.h>
#include "spur_reject.h"
void ui_spectrum_init(void);
void ui_spectrum_set_info(uint32_t center_hz, uint32_t span_hz, uint32_t rbw_hz);
void ui_spectrum_draw(const int8_t *spec, int n);
int  ui_spectrum_poll_event(void);
void ui_spectrum_handle(void);
#endif
