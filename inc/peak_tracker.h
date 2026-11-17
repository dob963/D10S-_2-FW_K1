#ifndef PEAK_TRACKER_H
#define PEAK_TRACKER_H
#include <stdint.h>
#include "spur_reject.h"
typedef struct { uint8_t previous[SPEC_BINS]; } peak_tracker_t;
typedef struct { uint8_t noise_floor, peak_level; int16_t peak_offset_q8, peak_bin; } peak_tracker_result_t;
void peak_tracker_init(peak_tracker_t *state);
void peak_tracker_update(peak_tracker_t *state,const int8_t *levels,int n,peak_tracker_result_t *result);
#endif
