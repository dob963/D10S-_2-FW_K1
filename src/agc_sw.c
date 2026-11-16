#include "agc_sw.h"
void agc_init(agc_t *a) {
    a->gain_q8=256; a->attack_q8=384; a->decay_q8=12;
    a->target_q8=180*256; a->min_q8=32*256; a->max_q8=1024*256;
    a->noise_floor_q8=40*256;
}
int16_t agc_process(agc_t *a, int16_t rssi_raw) {
    if (rssi_raw < ((a->noise_floor_q8>>8)+8))
        a->noise_floor_q8 += (((int32_t)rssi_raw<<8)-a->noise_floor_q8)>>6;
    int32_t y=((int32_t)rssi_raw*a->gain_q8)>>8;
    int32_t err=a->target_q8-(y<<8);
    if (err<0) a->gain_q8-=(a->gain_q8*a->attack_q8)>>10;
    else       a->gain_q8+=(a->gain_q8*a->decay_q8)>>12;
    if (a->gain_q8<a->min_q8) a->gain_q8=a->min_q8;
    if (a->gain_q8>a->max_q8) a->gain_q8=a->max_q8;
    y=((int32_t)rssi_raw*a->gain_q8)>>8;
    if (y>255) y=255;
    if (y<0) y=0;
    return (int16_t)y;
}
int32_t agc_get_noise_floor(const agc_t *a){return a->noise_floor_q8>>8;}
