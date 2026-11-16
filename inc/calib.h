#ifndef CALIB_H
#define CALIB_H
#include <stdint.h>
void calib_load(void);
int16_t calib_apply_dc(int16_t v,int bin);
#endif
