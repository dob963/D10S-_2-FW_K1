#ifndef SPUR_REJECT_H
#define SPUR_REJECT_H
#include <stdint.h>
#define SPEC_BINS 128
#define PERSIST_N 6
#define AFC_TOL 3
#define AMP_TOL 8
typedef struct {
    uint8_t persist[SPEC_BINS];
    int16_t prev_afc[SPEC_BINS];
    int16_t prev_amp[SPEC_BINS];
    int8_t spur_flag[SPEC_BINS];
} spur_t;
void spur_init(spur_t *s);
void spur_update(spur_t *s,const int16_t *spectrum,
                 const int16_t *afc,int8_t *out_clean);
#endif
