#include "config.h"
#include <string.h>


#ifdef PLATFORM_HOST
static uint8_t emulated_flash[1024];
#define CONFIG_FLASH_ADDR   emulated_flash
#define FLASH_ERASE()       memset(emulated_flash, 0xFF, sizeof(emulated_flash))
#define FLASH_WRITE(addr, data, size)   memcpy((void*)(addr), (data), (size))
#define FLASH_READ(addr, data, size)    memcpy((data), (const void*)(addr), (size))
#else

#include "gd32f4xx.h"          
#define CONFIG_FLASH_ADDR   (0x0807F800UL)   
static void flash_erase(void) {
    fmc_unlock();
    fmc_page_erase(CONFIG_FLASH_ADDR);
    fmc_lock();
}
static void flash_write(const void* data, size_t size) {
    const uint32_t* src = (const uint32_t*)data;
    size_t words = size / 4;
    fmc_unlock();
    fmc_page_erase(CONFIG_FLASH_ADDR);
    for (size_t i = 0; i < words; i++) {
        fmc_word_program(CONFIG_FLASH_ADDR + i * 4, src[i]);
    }
    fmc_lock();
}
static void flash_read(void* data, size_t size) {
    const uint8_t* src = (const uint8_t*)CONFIG_FLASH_ADDR;
    memcpy(data, src, size);
}
#define FLASH_ERASE()       flash_erase()
#define FLASH_WRITE(addr, data, size)   flash_write((data), (size))
#define FLASH_READ(addr, data, size)    flash_read((data), (size))
#endif


static const SystemConfig default_config = {
    .rms_periods = 1,
    .uart_baudrate = 115200,
    .adc_vref_mv = 3300.0f,
    .crc = 0
};

static SystemConfig current_config;
static uint8_t initialized = 0;

static uint16_t crc16(const uint8_t* data, size_t len) {
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

static int load_from_flash(SystemConfig* cfg) {
    SystemConfig temp;
    FLASH_READ(CONFIG_FLASH_ADDR, &temp, sizeof(SystemConfig));
    uint16_t saved_crc = temp.crc;
    temp.crc = 0;
    if (saved_crc == crc16((const uint8_t*)&temp, sizeof(temp))) {
        *cfg = temp;
        return 0;
    }
    return -1;
}

static void save_to_flash(const SystemConfig* cfg) {
    FLASH_WRITE(CONFIG_FLASH_ADDR, cfg, sizeof(SystemConfig));
}

void config_init(void) {
    if (initialized) return;
    if (load_from_flash(&current_config) != 0) {
        current_config = default_config;
        current_config.crc = 0;
        current_config.crc = crc16((const uint8_t*)&current_config, sizeof(current_config));
        save_to_flash(&current_config);
    }
    initialized = 1;
}

const SystemConfig* config_get(void) {
    return &current_config;
}

int config_set(const SystemConfig* cfg) {
    if (cfg->rms_periods < 1 || cfg->rms_periods > 10) return -1;
    if (cfg->uart_baudrate < 9600 || cfg->uart_baudrate > 921600) return -1;
    if (cfg->adc_vref_mv <= 0) return -1;
    current_config = *cfg;
    current_config.crc = 0;
    current_config.crc = crc16((const uint8_t*)&current_config, sizeof(current_config));
    save_to_flash(&current_config);
    return 0;
}