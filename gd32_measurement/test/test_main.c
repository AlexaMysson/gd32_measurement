#include "config.h"
#include <stdio.h>

int main(void) {
    config_init();
    const SystemConfig* cfg = config_get();

    printf("=== Config module test ===\n");
    printf("RMS periods:      %u\n", cfg->rms_periods);
    printf("UART baudrate:    %lu\n", (unsigned long)cfg->uart_baudrate);
    printf("ADC Vref (mV):    %.1f\n", cfg->adc_vref_mv);
    printf("CRC:              0x%04X\n", cfg->crc);
    printf("\nTest passed.\n");
    return 0;
}