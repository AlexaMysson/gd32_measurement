#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct {
    uint16_t rms_periods; 
    uint32_t uart_baudrate;  
    float adc_vref_mv;       
    uint16_t crc;            
} SystemConfig;

void config_init(void);
const SystemConfig* config_get(void);
int config_set(const SystemConfig *cfg);

#endif