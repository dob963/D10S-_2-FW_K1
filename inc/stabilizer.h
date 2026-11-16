#ifndef STABILIZER_H
#define STABILIZER_H
#include <stdint.h>
#include "spur_reject.h"
#define STAB_HI      8
#define STAB_LO      4
#define STAB_HOLD    3
#define STAB_RELEASE 2
#define STAB_DEAD    2
typedef struct {
    int8_t shown[SPEC_BINS];
    int8_t hold[SPEC_BINS];
} stabilizer_t;
void stabilizer_init(stabilizer_t *s);
void stabilizer_apply(stabilizer_t *s,const int8_t *in,int8_t *out,int n);
#endif
