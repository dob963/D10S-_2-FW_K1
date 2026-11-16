#ifndef BK4829_DRV_H
#define BK4829_DRV_H
#include <stdint.h>

/* Adattatore BK4829 UV-K1 V3.
 * Nessun registro BK4819 viene assunto compatibile.
 * Le letture RSSI/AFC e il tuning restano disabilitati finche' la mappa
 * registri specifica del BK4829 non e' disponibile da documentazione stock. */
void    bk4829_init(void);
void    bk4829_set_freq(uint32_t freq_hz);
void    bk4829_wait_settle(void);
int16_t bk4829_read_rssi(void);
int16_t bk4829_read_afc(void);

#endif
