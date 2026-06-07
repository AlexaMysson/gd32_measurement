#include "calc.h"
#include "meas.h"
#include "uart_out.h"
#include "config.h"
#include <stdio.h>
#include <unistd.h>  

int main(void) {
    config_init();
    uart_init();
    meas_init();
    meas_start();

    const SystemConfig* cfg = config_get();
    printf("Config: RMS periods=%u, Baudrate=%lu, Vref=%.1f mV\n",
        cfg->rms_periods, (unsigned long)cfg->uart_baudrate, cfg->adc_vref_mv);

    
    meas_set_signal(50.0f, 1.5f); 

    for (int i = 0; i < 10; i++) {
        size_t len;
        const uint16_t* samples = meas_get_current_buffer(&len);
        if (samples) {
            CalcResult result;
            int status = calc_process_samples(samples, len, cfg->adc_vref_mv, &result);
            if (status == 0) {
                uart_send_result(&result);
            }
            else {
                uart_send_string("Calculation error\n");
            }
            meas_buffer_processed();
        }
        sleep(1);  
    }
    return 0;
}