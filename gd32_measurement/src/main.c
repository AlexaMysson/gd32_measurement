#include "config.h"
#include "meas.h"
#include "calc.h"
#include <stdio.h>
#include <unistd.h>   

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#define SLEEP_MS(ms) usleep((ms)*1000)
#endif

int main(void) {

    config_init();
    meas_init();

    const SystemConfig* cfg = config_get();

    meas_start();

    int freq_steps = 0;
    int demo_counter = 0;
    float demo_freq;

    CalcResult result;
    while (1) {
        size_t len;
        const uint16_t* samples = meas_get_current_buffer(&len);
        if (samples) {
            int status = calc_process_samples(samples, len, cfg->adc_vref_mv, &result);
            if (status == 0 && result.valid) {
                printf("RMS=%.3f V, Freq=%.2f Hz, dF/dt=%.2f Hz/s, 1stHarm=%.3f V\n",
                    result.rms, result.frequency, result.df_dt, result.first_harmonic_amp);
            }
            else {
                printf("Calculation failed (insufficient data)\n");
            }
            meas_buffer_processed();
        }

        demo_counter++;
        if (demo_counter >= 20) {  
            demo_counter = 0;
            if (freq_steps < 10) {
                demo_freq += 1.0f;
                if (demo_freq > 55.0f) demo_freq = 55.0f;
            }
            else if (freq_steps < 20) {
                demo_freq -= 1.0f;
                if (demo_freq < 45.0f) demo_freq = 45.0f;
            }
            else {
                freq_steps = 0;
                demo_freq = 45.0f;
            }
            meas_set_signal(demo_freq, 1.5f);
            freq_steps++;
            if (freq_steps > 20) freq_steps = 0;
        }

    }
    return 0;
}