#ifndef WATERFALL_H
#define WATERFALL_H

#include <stdint.h>

/* Controllo indipendente: disabilitare il waterfall non interrompe lo sweep,
 * l'acquisizione RSSI o il disegno dello spettro live.
 */
void waterfall_set_enabled(int enabled);
int  waterfall_is_enabled(void);
void waterfall_clear(void);
void waterfall_push(const int8_t *spec, int n);

#endif
