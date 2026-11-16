#include "ui_spectrum.h"
#include "st7565.h"

#define GRAPH_TOP 32
#define GRAPH_HEIGHT 32

static int8_t peak[SPEC_BINS];
static uint32_t center_hz;
static uint32_t span_hz;
static uint32_t rbw_hz;

static void put3(char *p, uint32_t v)
{
    p[0] = (char)('0' + (v / 100u) % 10u);
    p[1] = (char)('0' + (v / 10u) % 10u);
    p[2] = (char)('0' + v % 10u);
}

static void put_mhz(char *p, uint32_t hz)
{
    uint32_t mhz = hz / 1000000u;
    uint32_t frac = (hz % 1000000u) / 1000u;
    if (mhz > 999u) mhz = 999u;
    put3(p, mhz);
    p[3] = '.';
    put3(p + 4, frac);
    p[7] = '\0';
}

void ui_spectrum_init(void)
{
    for (int i = 0; i < SPEC_BINS; ++i) peak[i] = 0;
    center_hz = 146000000u;
    span_hz = 4000000u;
    rbw_hz = 0u;
}

void ui_spectrum_set_info(uint32_t center, uint32_t span, uint32_t rbw)
{
    center_hz = center;
    span_hz = span;
    rbw_hz = rbw;
}

void ui_spectrum_draw(const int8_t *spec, int n)
{
    if (!spec || n <= 0) return;
    if (n > SPEC_BINS) n = SPEC_BINS;

    st7565_clear();

    /* Intestazione compatta: frequenza centrale e span in MHz;
     * RBW in kHz. Valori solo informativi, non modificano lo sweep.
     */
    char c[8], s[8], line[24];
    put_mhz(c, center_hz);
    put_mhz(s, span_hz);
    line[0]='C'; line[1]=' '; 
    for (int i=0;i<7;++i) line[2+i]=c[i];
    line[9]=' '; line[10]='S'; line[11]=' ';
    for (int i=0;i<7;++i) line[12+i]=s[i];
    line[19]='\0';
    st7565_draw_text(0,0,line);

    uint32_t rbw_k = rbw_hz / 1000u;
    if (rbw_k > 999u) rbw_k=999u;
    char rbwline[12]={'R','B','W',' ',0,0,0,'K','H','z',0,0};
    put3(rbwline+4,rbw_k);
    st7565_draw_text(0,8,rbwline);

    int max_bin = 0;
    int max_val = -1;
    for (int i=0;i<n;++i) {
        int live = spec[i] < 0 ? 0 : spec[i];
        if (live > 255) live = 255;

        /* Peak Hold visuale: separato dalla traccia live. */
        if (live > peak[i]) peak[i]=(int8_t)live;
        else if (peak[i] > 0) --peak[i];

        int x0 = (i * ST7565_WIDTH) / n;
        int x1 = ((i + 1) * ST7565_WIDTH) / n - 1;
        if (x1 < x0) x1=x0;
        int h = (live * GRAPH_HEIGHT) / 255;
        int ph = ((int)peak[i] * GRAPH_HEIGHT) / 255;
        int base = ST7565_HEIGHT - 1;
        int top = base - h + 1;
        int ptop = base - ph + 1;

        /* Barre live disegnate direttamente: nessuna fusione tra sweep,
         * nessun clamp di gradiente e nessuna modifica dei campioni.
         */
        for (int x=x0; x<=x1; ++x) {
            if (h > 0) st7565_draw_vline(x,top,base);
            if (ph > 0) st7565_set_pixel(x,ptop,1);
        }
        if (live > max_val) { max_val=live; max_bin=i; }
    }

    /* Marcatore del massimo: piccolo triangolo sopra la barra. */
    if (max_val > 0) {
        int mx = (max_bin * ST7565_WIDTH) / n;
        int mh = (max_val * GRAPH_HEIGHT) / 255;
        int my = ST7565_HEIGHT - 1 - mh;
        if (my > GRAPH_TOP + 2) {
            st7565_set_pixel(mx,my-2,1);
            st7565_set_pixel(mx-1,my-1,1);
            st7565_set_pixel(mx+1,my-1,1);
        }
    }
}

int ui_spectrum_poll_event(void) { return 0; }
void ui_spectrum_handle(void) { }
