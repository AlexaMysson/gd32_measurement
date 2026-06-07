#include "unity.h"
#include "config.h"
#include <string.h>

#ifdef PLATFORM_HOST
#undef CONFIG_FLASH_ADDR
static uint8_t emulated_flash[1024];
#define CONFIG_FLASH_ADDR emulated_flash
#endif

static void flash_erase(void) {
    memset(emulated_flash, 0xFF, sizeof(emulated_flash));
}

void setUp(void) {
    flash_erase();
}

void tearDown(void) {}

void test_config_defaults_after_init(void) {
    config_init();
    const SystemConfig* cfg = config_get();
    TEST_ASSERT_EQUAL_UINT16(1, cfg->rms_periods);
    TEST_ASSERT_EQUAL_UINT32(115200, cfg->uart_baudrate);
    TEST_ASSERT_EQUAL_FLOAT(3300.0f, cfg->adc_vref_mv);
    TEST_ASSERT_NOT_EQUAL(0, cfg->crc);
}

void test_config_save_and_load(void) {
    config_init(); 

    SystemConfig new_cfg;
    new_cfg.rms_periods = 3;
    new_cfg.uart_baudrate = 9600;
    new_cfg.adc_vref_mv = 2500.0f;
    new_cfg.crc = 0;

    int ret = config_set(&new_cfg);
    TEST_ASSERT_EQUAL_INT(0, ret);

    const SystemConfig* cfg = config_get();
    TEST_ASSERT_EQUAL_UINT16(3, cfg->rms_periods);
    TEST_ASSERT_EQUAL_UINT32(9600, cfg->uart_baudrate);
    TEST_ASSERT_EQUAL_FLOAT(2500.0f, cfg->adc_vref_mv);
}

void test_config_invalid_params(void) {
    config_init();
    SystemConfig bad;
    bad.rms_periods = 0;
    bad.uart_baudrate = 115200;
    bad.adc_vref_mv = 3300.0f;
    TEST_ASSERT_EQUAL_INT(-1, config_set(&bad));

    bad.rms_periods = 1;
    bad.uart_baudrate = 100; 
    TEST_ASSERT_EQUAL_INT(-1, config_set(&bad));

    bad.uart_baudrate = 115200;
    bad.adc_vref_mv = -1.0f;
    TEST_ASSERT_EQUAL_INT(-1, config_set(&bad));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_config_defaults_after_init);
    RUN_TEST(test_config_save_and_load);
    RUN_TEST(test_config_invalid_params);
    return UNITY_END();
}