#include "config.h"
#include <string.h>

#define CONFIG_FLASH_ADDR  0x0807F800UL

static const SystemConfig default_config = {
    .rms_periods = 1,
    .uart_baudrate = 115200,
    .adc_vref_mv = 3300.0f,
    .crc = 0
};

static SystemConfig current_config;
static int initialized = 0;

static uint16_t calculate_crc(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

#ifdef PLATFORM_HOST ///Эмуляция Flash для тестов на ПК
static uint8_t emulated_flash[1024];
#define FLASH_PTR emulated_flash
#else
#define FLASH_PTR ((uint8_t*)CONFIG_FLASH_ADDR)
#endif

static int config_load_from_flash(SystemConfig* cfg) {
    const SystemConfig* flash_cfg = (const SystemConfig*)FLASH_PTR;

    SystemConfig temp = *flash_cfg;
    uint16_t saved_crc = temp.crc;
    temp.crc = 0;

    if (saved_crc != calculate_crc((const uint8_t*)&temp, sizeof(temp)))
        return -1;

    *cfg = temp;
    return 0;
}

static void config_save_to_flash(const SystemConfig* cfg) {
#ifdef PLATFORM_HOST
    memcpy(emulated_flash, cfg, sizeof(SystemConfig));
#else
    // Запись во Flash для GD32 будет добавлена позже
#endif
}

void config_init(void) {
    if (initialized) return;

    if (config_load_from_flash(&current_config) != 0) {
        current_config = default_config;
        current_config.crc = 0;
        current_config.crc = calculate_crc((const uint8_t*)&current_config, sizeof(current_config));
        config_save_to_flash(&current_config);
    }

    initialized = 1;
}

const SystemConfig* config_get(void) {
    return &current_config;
}

int config_set(const SystemConfig* cfg) {
    if (cfg->rms_periods < 1 || cfg->rms_periods > 10) return -1;
    if (cfg->uart_baudrate < 9600 || cfg->uart_baudrate > 921600) return -1;
    if (cfg->adc_vref_mv <= 0.0f) return -1;

    current_config = *cfg;
    current_config.crc = 0;
    current_config.crc = calculate_crc((const uint8_t*)&current_config, sizeof(current_config));
    config_save_to_flash(&current_config);
    return 0;
}