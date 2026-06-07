#include "meas.h"
#include <math.h>
#include <stdio.h>

#define BUFFER_SIZE 2048     
#define FS 19200.0f           
#define VREF_MV 3300.0f      
#define DC_OFFSET_V 1.65f    

static uint16_t buffer[BUFFER_SIZE];
static size_t sample_index = 0;   
static int buffer_ready = 0;      

static float signal_freq = 50.0f;
static float signal_amp = 1.5f;

void meas_init(void) {
        (int)FS, VREF_MV, DC_OFFSET_V);
}

void meas_start(void) {
    sample_index = 0;
    buffer_ready = 0;
}

void meas_set_signal(float frequency_hz, float amplitude_v) {
    signal_freq = frequency_hz;
    signal_amp = amplitude_v;
}

const uint16_t* meas_get_current_buffer(size_t* len) {
    if (!buffer_ready) {
        for (size_t i = 0; i < BUFFER_SIZE; i++) {
            double t = (sample_index + i) / FS;
            double v = signal_amp * sin(2.0 * M_PI * signal_freq * t);
            double adc_v = v + DC_OFFSET_V;
            int code = (int)(adc_v * 4095.0 / (VREF_MV / 1000.0));
            if (code < 0) code = 0;
            if (code > 4095) code = 4095;
            buffer[i] = (uint16_t)code;
        }
        sample_index += BUFFER_SIZE;
        buffer_ready = 1;
    }
    *len = BUFFER_SIZE;
    return buffer;
}

void meas_buffer_processed(void) {
    buffer_ready = 0; 
}