#include "st7565.h"
#include <stdint.h>

/* Bus display dedicato: PA5=SCK, PA7=MOSI, PA6=A0, PB2=CS.
 * Il bus RF resta su PB8/PB9/PF9 (hw_init.c): i pin non coincidono.
 * RESET hardware del modulo LCD non identificato: usare il reset software
 * ST7565 E2; verificare il reset fisico sullo schema/PCB prima del test.
 */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define LCD_SCK 5u
#define LCD_A0 6u
#define LCD_MOSI 7u
#define LCD_CS 2u
extern void delay_ms(uint32_t ms);

static void lcd_pin(uint32_t bsrr, uint32_t pin, int high)
{
    REG32(bsrr) = high ? (1u << pin) : (1u << (pin + 16u));
}

static void lcd_send_byte(uint8_t value)
{
    for (int bit = 7; bit >= 0; --bit) {
        lcd_pin(0x50000018u, LCD_MOSI, (value >> bit) & 1u);
        /* SPI mode 3: SCK alto a riposo, campionamento sul secondo fronte
         * (salita). Configurazione rilevata nel driver upstream K1 ST7565. */
        lcd_pin(0x50000018u, LCD_SCK, 0);
        __asm volatile ("nop");
        lcd_pin(0x50000018u, LCD_SCK, 1);
        __asm volatile ("nop");
    }
}

static void lcd_command(uint8_t value)
{
    lcd_pin(0x50000418u, LCD_CS, 0);
    lcd_pin(0x50000018u, LCD_A0, 0);
    lcd_send_byte(value);
    lcd_pin(0x50000418u, LCD_CS, 1);
}

static void lcd_command_pair(uint8_t command, uint8_t parameter)
{
    lcd_pin(0x50000418u, LCD_CS, 0);
    lcd_pin(0x50000018u, LCD_A0, 0);
    lcd_send_byte(command);
    lcd_send_byte(parameter);
    lcd_pin(0x50000418u, LCD_CS, 1);
}

/* 1 KB di framebuffer: 8 pagine x 128 colonne, formato ST7565. */
static uint8_t fb[ST7565_FB_SIZE];

/* Font 5x7 minimale: cifre, lettere maiuscole e punteggiatura usata
 * dalla schermata. Ogni glifo e' memorizzato per colonne, bit LSB in alto.
 */
static const uint8_t font_num[10][5] = {
 {0x3e,0x51,0x49,0x45,0x3e},{0x00,0x42,0x7f,0x40,0x00},
 {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4b,0x31},
 {0x18,0x14,0x12,0x7f,0x10},{0x27,0x45,0x45,0x45,0x39},
 {0x3c,0x4a,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
 {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1e}
};
static const uint8_t font_alpha[26][5] = {
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},
 {0x3e,0x41,0x41,0x41,0x22},{0x7f,0x41,0x41,0x22,0x1c},
 {0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},
 {0x00,0x41,0x7f,0x41,0x00},{0x20,0x40,0x41,0x3f,0x01},
 {0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,0x02,0x0c,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},
 {0x3e,0x41,0x41,0x41,0x3e},{0x7f,0x09,0x09,0x09,0x06},
 {0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},
 {0x3f,0x40,0x40,0x40,0x3f},{0x1f,0x20,0x40,0x20,0x1f},
 {0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43}
};

void st7565_init(void)
{
    st7565_clear();
    /* Sequenza standard ST7565: reset software, display off, bias,
     * ADC/COM scan, power control, rapporto resistivo e contrasto.
     * Bias, orientamento e contrasto sono valori iniziali da verificare
     * sul pannello reale; non sono registri BK4829.
     */
    lcd_pin(0x50000418u, LCD_CS, 1);
    lcd_pin(0x50000018u, LCD_SCK, 1); /* clock idle alto: SPI mode 3 */
    lcd_pin(0x50000018u, LCD_A0, 0);
    lcd_command(0xAEu); /* display OFF prima del reset software */
    lcd_command(0xE2u); /* reset software; K1 non espone reset hardware LCD */
    delay_ms(120u);

    /* Parametri derivati dal driver K1 ST7565 upstream:
     * App/driver/st7565.c: bias 1/9, COM normale, SEG invertito,
     * EV 31, resistor ratio 5.0. Reimplementazione autonoma.
     */
    lcd_command(0xA2u); /* bias 1/9 */
    lcd_command(0xC0u); /* COM normale */
    lcd_command(0xA1u); /* SEG invertito */
    lcd_command(0xA6u); /* display normale */
    lcd_command(0xA4u); /* RAM normale */
    lcd_command(0x24u); /* resistor ratio 5.0 */
    lcd_command_pair(0x81u, 31u); /* contrasto iniziale 31/63 */

    /* Sequenza di accensione del circuito interno: booster/regolatore/follower. */
    lcd_command(0x2Bu);
    delay_ms(1u);
    lcd_command(0x2Eu);
    delay_ms(1u);
    for (uint8_t i=0; i<4u; ++i) lcd_command(0x2Fu);
    delay_ms(40u);

    lcd_command(0x40u); /* start line 0 */
    st7565_blit_full(); /* azzera le 8 pagine RAM prima di accendere */
    lcd_command(0xAFu); /* display ON */
}

void st7565_clear(void)
{
    for (unsigned i = 0; i < ST7565_FB_SIZE; ++i) fb[i] = 0u;
}

void st7565_set_pixel(int x, int y, int on)
{
    if (x < 0 || x >= ST7565_WIDTH || y < 0 || y >= ST7565_HEIGHT) return;
    unsigned i = (unsigned)(y >> 3) * ST7565_WIDTH + (unsigned)x;
    uint8_t mask = (uint8_t)(1u << (y & 7));
    if (on) fb[i] |= mask;
    else fb[i] &= (uint8_t)~mask;
}

void st7565_draw_hline(int x0, int x1, int y)
{
    if (x0 > x1) { int t=x0; x0=x1; x1=t; }
    for (int x=x0; x<=x1; ++x) st7565_set_pixel(x,y,1);
}

void st7565_draw_vline(int x, int y0, int y1)
{
    if (y0 > y1) { int t=y0; y0=y1; y1=t; }
    for (int y=y0; y<=y1; ++y) st7565_set_pixel(x,y,1);
}

void st7565_draw_char(int x, int y, char c)
{
    const uint8_t *g = 0;
    if (c >= '0' && c <= '9') g = font_num[(unsigned)(c-'0')];
    else if (c >= 'A' && c <= 'Z') g = font_alpha[(unsigned)(c-'A')];
    else if (c >= 'a' && c <= 'z') g = font_alpha[(unsigned)(c-'a')];
    if (g) {
        for (int col=0; col<5; ++col)
            for (int row=0; row<7; ++row)
                if (g[col] & (1u<<row)) st7565_set_pixel(x+col,y+row,1);
    } else if (c=='.') st7565_set_pixel(x+2,y+6,1);
    else if (c==':') { st7565_set_pixel(x+2,y+2,1); st7565_set_pixel(x+2,y+5,1); }
    else if (c=='-') st7565_draw_hline(x+1,x+3,y+3);
}

void st7565_draw_text(int x, int y, const char *s)
{
    if (!s) return;
    while (*s) { st7565_draw_char(x,y,*s++); x += 6; if (x >= ST7565_WIDTH) break; }
}

void st7565_blit_full(void)
{
    for (uint8_t page = 0; page < 8u; ++page) {
        lcd_pin(0x50000418u, LCD_CS, 0);
        lcd_pin(0x50000018u, LCD_A0, 0);
        lcd_send_byte((uint8_t)(0xB0u | page));
        lcd_send_byte(0x10u); /* colonna alta = 0 */
        lcd_send_byte(0x04u); /* offset colonna 4: glass K1 usa RAM 132-col */
        lcd_pin(0x50000018u, LCD_A0, 1);
        for (uint8_t col = 0; col < 128u; ++col)
            lcd_send_byte(fb[(uint16_t)page * 128u + col]);
        lcd_pin(0x50000418u, LCD_CS, 1);
    }
}
