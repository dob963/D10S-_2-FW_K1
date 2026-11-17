#ifndef SWEEP_ENGINE_H
#define SWEEP_ENGINE_H
#include <stdint.h>
#include "spur_reject.h"
/* valid_out[i]=1 solo se il bin i è stato realmente campionato. */
void sweep_adaptive(uint32_t f_start,uint32_t f_stop,
                    int16_t *spectrum_out,int16_t *afc_out,
                    uint8_t *valid_out);
/* EMA disponibile per altri usi, non usare nel percorso live Spectrum. */
void sweep_average(const int16_t *in,int16_t *acc,int n,int alpha_q8);
#endif
