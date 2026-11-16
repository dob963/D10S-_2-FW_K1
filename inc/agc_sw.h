#ifndef AGC_SW_H
#define AGC_SW_H
#include <stdint.h>
typedef struct {
    int32_t gain_q8, attack_q8, decay_q8, target_q8, min_q8, max_q8;
    int32_t noise_floor_q8;
} agc_t;
void agc_init(agc_t *a);
int16_t agc_process(agc_t *a, int16_t rssi_raw);
int32_t agc_get_noise_floor(const agc_t *a);
#endif
