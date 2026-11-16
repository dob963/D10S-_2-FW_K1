#ifndef UI_SPECTRUM_H
#define UI_SPECTRUM_H
#include <stdint.h>
#include "spur_reject.h"
void ui_spectrum_init(void);
void ui_spectrum_draw(const int8_t *spec,int n);
int ui_spectrum_poll_event(void);
void ui_spectrum_handle(void);
void ui_spectrum_clamp_gradient(int8_t *spec,int n,int max_delta);
void ui_spectrum_crossfade(const int8_t *new_spec,int n,int alpha_q8);
#endif
