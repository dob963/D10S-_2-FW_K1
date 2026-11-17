#include "ui_spectrum.h"
#include "st7565.h"
#include <stdint.h>

#define GRAPH_TOP 32
#define GRAPH_HEIGHT 32
#define KEY_INVALID 0
#define KEY_MENU 1
#define KEY_UP 2
#define KEY_DOWN 3
#define KEY_EXIT 4
#define KEY_STAR 5
#define KEY_F 6
#define KEY_SIDE1 7
#define KEY_SIDE2 8
#define KEY_PTT 9

static int8_t peak[SPEC_BINS];
static uint32_t center_hz;
static uint32_t span_hz;
static uint32_t rbw_hz;
static uint8_t peak_hold = 1u;
static int pending_event;
static int stable_key;
static int reported_key;
static uint8_t stable_count;
static const uint32_t min_center = 144000000u;
static const uint32_t max_center = 148000000u;
static const uint32_t spans[] = {500000u, 1000000u, 2000000u, 4000000u};
static uint8_t span_index = 3u;
static uint32_t detected_frequency;
static uint8_t detected_level, detected_noise;

/* GPIO access mirrors the verified UV-K1 matrix wiring, without importing
 * the upstream keyboard implementation. */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define GPIOB_IDR REG32(0x50000410u)
#define GPIOB_BSRR REG32(0x50000418u)
static const uint8_t col_pin[4] = {6u,5u,4u,3u};

static void columns_idle(void)
{
    for (unsigned i=0;i<4u;++i) GPIOB_BSRR = 1u << col_pin[i];
}
static void select_column(unsigned col)
{
    columns_idle();
    if (col < 4u) GPIOB_BSRR = 1u << (col_pin[col] + 16u);
}
static uint8_t read_rows(void)
{
    uint8_t raw=(uint8_t)((~(GPIOB_IDR >> 12u)) & 0x0Fu);
    /* Row 0 is PB15, row 3 is PB12 in the UV-K1 matrix. */
    return (uint8_t)(((raw&0x01u)<<3)|((raw&0x02u)<<1)|((raw&0x04u)>>1)|((raw&0x08u)>>3));
}
static int decode_key(unsigned col, unsigned row)
{
    static const int map[5][4] = {
        {KEY_SIDE1,KEY_SIDE2,KEY_INVALID,KEY_INVALID},
        {KEY_MENU,10,13,16},
        {KEY_UP,11,14,17},
        {KEY_DOWN,12,15,18},
        {KEY_EXIT,KEY_STAR,9,KEY_F}
    };
    if (col > 4u || row > 3u) return KEY_INVALID;
    return map[col][row];
}
static int scan_key(void)
{
    int found = KEY_INVALID;
    for (unsigned col=0; col<5u; ++col) {
        select_column(col == 0u ? 4u : col-1u);
        /* 4 identical samples suppress transient GPIO noise. */
        uint8_t rows = read_rows();
        unsigned same = 0u;
        for (unsigned k=0;k<4u;++k) {
            uint8_t next = read_rows();
            if (next == rows) ++same;
            else { rows = next; same = 0u; }
        }
        if (same < 3u) continue;
        for (unsigned row=0;row<4u;++row) {
            if (rows & (1u<<row)) {
                found = decode_key(col,row);
                if (found != KEY_INVALID) { columns_idle(); return found; }
            }
        }
    }
    columns_idle();
    /* PTT is not a spectrum control; don't interpret it as a key. */
    return found;
}

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
    put3(p, mhz); p[3] = '.'; put3(p + 4, frac); p[7] = '\0';
}
static void clamp_center_to_span(void)
{
    uint32_t half=span_hz/2u;
    uint32_t lo=min_center+half, hi=max_center-half;
    if(center_hz<lo)center_hz=lo;
    if(center_hz>hi)center_hz=hi;
}
void ui_spectrum_init(void)
{
    for (int i=0;i<SPEC_BINS;++i) peak[i]=0;
    center_hz=146000000u; span_index=3u; span_hz=spans[span_index];
    rbw_hz=0u; peak_hold=1u; pending_event=0;
    stable_key=reported_key=KEY_INVALID; stable_count=0u;
}
void ui_spectrum_set_info(uint32_t center,uint32_t span,uint32_t rbw)
{
    center_hz=center; span_hz=span; rbw_hz=rbw; clamp_center_to_span();
}
void ui_spectrum_set_detection(uint32_t frequency,uint8_t level,uint8_t noise_floor)
{
    detected_noise=noise_floor;
    if(frequency) { detected_frequency=frequency; detected_level=level; }
    else if(!peak_hold) { detected_frequency=0u; detected_level=0u; }
}
void ui_spectrum_get_range(uint32_t *start,uint32_t *stop)
{
    uint32_t half=span_hz/2u;
    uint32_t lo=center_hz-half, hi=center_hz+half;
    if (lo < min_center) { hi += min_center-lo; lo=min_center; }
    if (hi > max_center) { lo -= hi-max_center; hi=max_center; }
    if (start) *start=lo;
    if (stop) *stop=hi;
}
void ui_spectrum_draw(const int8_t *spec,int n)
{
    if (!spec || n<=0) return;
    if (n>SPEC_BINS) n=SPEC_BINS;
    st7565_clear();
    char c[8],s[8],line[24];
    put_mhz(c,center_hz); put_mhz(s,span_hz);
    line[0]='C'; line[1]=' ';
    for(int i=0;i<7;++i) line[2+i]=c[i];
    line[9]=' '; line[10]='S'; line[11]=' ';
    for(int i=0;i<7;++i) line[12+i]=s[i];
    line[19]='\0'; st7565_draw_text(0,0,line);
    uint32_t rbw_k=rbw_hz/1000u; if(rbw_k>999u) rbw_k=999u;
    char rbwline[12]={'R','B','W',' ',0,0,0,'K','H','z',0,0};
    if(!rbw_hz) {rbwline[4]='-';rbwline[5]='-';rbwline[6]='-';}
    else put3(rbwline+4,rbw_k);
    st7565_draw_text(0,8,rbwline);
    char detline[24]={'N','F',' ',0,0,0,' ','P','K',' ',0,0,0,'.',0,0,0,0,0,0,0,0,0};
    put3(detline+3,detected_noise);
    if(detected_frequency){uint32_t mhz=detected_frequency/1000000u,frac=(detected_frequency%1000000u)/1000u;put3(detline+10,mhz);put3(detline+14,frac);}
    else{detline[10]='-';detline[11]='-';detline[12]='-';detline[14]='-';detline[15]='-';detline[16]='-';}
    (void)detected_level; st7565_draw_text(0,16,detline);
    for(int i=0;i<n;++i) {
        int live=spec[i]<0?0:spec[i]; if(live>127) live=127;
        if(peak_hold) {
            if(live>peak[i]) peak[i]=(int8_t)live;
            else if(peak[i]>0) --peak[i];
        } else peak[i]=(int8_t)live;
        int x0=(i*ST7565_WIDTH)/n;
        int x1=((i+1)*ST7565_WIDTH)/n-1; if(x1<x0)x1=x0;
        int h=(live*GRAPH_HEIGHT)/127;
        int ph=((int)peak[i]*GRAPH_HEIGHT)/127;
        int base=ST7565_HEIGHT-1,top=base-h+1,ptop=base-ph+1;
        for(int x=x0;x<=x1;++x) {
            if(h>0) st7565_draw_vline(x,top,base);
            if(ph>0) st7565_set_pixel(x,ptop,1);
        }
    }
    if(detected_frequency && span_hz) {
        uint32_t start=center_hz-span_hz/2u;
        uint32_t stop=center_hz+span_hz/2u;
        if(detected_frequency>=start && detected_frequency<=stop) {
            int mx=(int)(((uint64_t)(detected_frequency-start)*(ST7565_WIDTH-1u))/span_hz);
            int mh=((int)detected_level*GRAPH_HEIGHT)/127;
            int my=ST7565_HEIGHT-1-mh;
            if(my>GRAPH_TOP+2) {
                st7565_set_pixel(mx,my-2,1);
                st7565_set_pixel(mx-1,my-1,1);
                st7565_set_pixel(mx+1,my-1,1);
            }
        }
    }
}
int ui_spectrum_poll_event(void)
{
    int key=scan_key();
    if(key==stable_key) { if(stable_count<4u) ++stable_count; }
    else { stable_key=key; stable_count=1u; }
    if(stable_count>=3u && key!=reported_key) {
        reported_key=key;
        if(key!=KEY_INVALID) { pending_event=key; return 1; }
    }
    if(key==KEY_INVALID && stable_count>=3u) reported_key=KEY_INVALID;
    return 0;
}
void ui_spectrum_handle(void)
{
    int key=pending_event; pending_event=0;
    switch(key) {
    case KEY_UP:
        if(center_hz+25000u<=max_center-span_hz/2u) center_hz+=25000u;
        break;
    case KEY_DOWN:
        if(center_hz>=min_center+span_hz/2u+25000u) center_hz-=25000u;
        break;
    case KEY_MENU:
        span_index=(uint8_t)((span_index+1u)%4u);
        span_hz=spans[span_index];
        clamp_center_to_span();
        break;
    case KEY_EXIT:
        center_hz=146000000u; span_index=3u; span_hz=spans[span_index];
        clamp_center_to_span();
        break;
    case KEY_F:
        peak_hold=(uint8_t)!peak_hold;
        break;
    case KEY_STAR:
        for(int i=0;i<SPEC_BINS;++i) peak[i]=0;
        detected_frequency=0u; detected_level=0u;
        break;
    case KEY_SIDE1:
        if(span_index>0u) --span_index;
        span_hz=spans[span_index];
        clamp_center_to_span();
        break;
    case KEY_SIDE2:
        if(span_index<3u) ++span_index;
        span_hz=spans[span_index];
        clamp_center_to_span();
        break;
    default:
        break;
    }
}
