#ifndef ST7565_H
#define ST7565_H

#include <stdint.h>

#define ST7565_WIDTH 128
#define ST7565_HEIGHT 64
#define ST7565_FB_SIZE 1024

/* Framebuffer 128x64 e trasporto bit-bang ST7565 dedicato.
 * Inizializzazione corrente: DISPLAY OFF, reset software 0xE2,
 * attesa 120 ms, configurazione controller e DISPLAY ON.
 * Il pin fisico RES non e' assegnato a un GPIO: la connessione sulla
 * revisione PCB resta da verificare. Non pilotare un pin ipotetico.
 */
void st7565_init(void);
void st7565_clear(void);
void st7565_set_pixel(int x, int y, int on);
void st7565_draw_hline(int x0, int x1, int y);
void st7565_draw_vline(int x, int y0, int y1);
void st7565_blit_full(void);
void st7565_draw_char(int x, int y, char c);
void st7565_draw_text(int x, int y, const char *s);

#endif
