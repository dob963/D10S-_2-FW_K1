#include "spur_reject.h"
#include "sweep_engine.h"
#include "bk4829_drv.h"
#include "median3.h"
#include "waterfall.h"
#include "ui_spectrum.h"
#include "stabilizer.h"
#include "hw_init.h"
#include "calib.h"
#include "st7565.h"
#include "peak_tracker.h"

static int16_t spec_raw[SPEC_BINS], spec_avg[SPEC_BINS];
static int16_t afc_raw[SPEC_BINS];
static uint8_t spec_valid[SPEC_BINS];
static int8_t spec_clean[SPEC_BINS], spec_stable[SPEC_BINS];
static stabilizer_t stab;
static peak_tracker_t peak_state;
static peak_tracker_result_t peak_result;

int main(void)
{
    hw_init();
    calib_load();
    stabilizer_init(&stab);
    peak_tracker_init(&peak_state);
    ui_spectrum_init();
    ui_spectrum_set_info(146000000u,4000000u,0u);

    for (;;) {
        uint32_t f_start, f_stop;
        ui_spectrum_get_range(&f_start,&f_stop);
        sweep_adaptive(f_start,f_stop,spec_raw,afc_raw,spec_valid);
        for(int i=0;i<SPEC_BINS;++i) spec_avg[i]=spec_raw[i];
        for(int i=1;i<SPEC_BINS-1;++i) {
            if(spec_valid[i-1]&&spec_valid[i]&&spec_valid[i+1])
                spec_avg[i]=median3(spec_raw[i-1],spec_raw[i],spec_raw[i+1]);
        }
        for(int i=0;i<SPEC_BINS;++i) {
            if(!spec_valid[i]) {spec_clean[i]=0;continue;}
            int32_t level=(spec_avg[i]>0)?(spec_avg[i]/4):0;
            level=calib_apply_dc((int16_t)level,i);
            if(level>127)level=127;
            spec_clean[i]=(int8_t)level;
        }
        stabilizer_apply(&stab,spec_clean,spec_stable,SPEC_BINS);
        peak_tracker_update(&peak_state,spec_stable,SPEC_BINS,&peak_result);
        uint32_t peak_frequency=0u;
        if(peak_result.peak_bin>=0){int64_t width=(int64_t)(f_stop-f_start);int64_t f=(int64_t)f_start+(width*peak_result.peak_bin)/(SPEC_BINS-1)+(width*peak_result.peak_offset_q8)/(256*(SPEC_BINS-1));if(f>=(int64_t)f_start&&f<=(int64_t)f_stop)peak_frequency=(uint32_t)f;}
        ui_spectrum_set_detection(peak_frequency,peak_result.peak_level,peak_result.noise_floor);
        ui_spectrum_draw(spec_stable,SPEC_BINS);
        waterfall_push(spec_clean,SPEC_BINS);
        st7565_blit_full();
        delay_ms(67u);
        if(ui_spectrum_poll_event()) {
            ui_spectrum_handle();
            ui_spectrum_get_range(&f_start,&f_stop);
            ui_spectrum_set_info((f_start+f_stop)/2u,f_stop-f_start,0u);
        }
    }
}
