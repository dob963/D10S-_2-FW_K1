#ifndef SWEEP_ENGINE_H
#define SWEEP_ENGINE_H
#include <stdint.h>
#include "spur_reject.h"
void sweep_adaptive(uint32_t f_start,uint32_t f_stop,
                    int16_t *spectrum_out,int16_t *afc_out);
void sweep_average(const int16_t *in,int16_t *acc,int n,int alpha_q8);
#endif
