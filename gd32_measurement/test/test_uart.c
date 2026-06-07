#include "unity.h"
#include "uart_out.h"
#include "calc.h"


void setUp(void) {
    
}

void tearDown(void) {}

void test_uart_send_result_valid(void) {
    CalcResult res = {
        .rms = 1.234f,
        .frequency = 50.123f,
        .df_dt = 0.567f,
        .first_harmonic_amp = 1.500f,
        .valid = 1
    };
    uart_send_result(&res);
    extern char last_output[256];
    extern int output_len;
    TEST_ASSERT_NOT_EQUAL(0, output_len);
    TEST_ASSERT_TRUE(strstr(last_output, "1.234") != NULL);
    TEST_ASSERT_TRUE(strstr(last_output, "50.123") != NULL);
    TEST_ASSERT_TRUE(strstr(last_output, "0.567") != NULL);
    TEST_ASSERT_TRUE(strstr(last_output, "1.500") != NULL);
}

void test_uart_send_result_invalid(void) {
    CalcResult res = { .valid = 0 };
    uart_send_result(&res);
    extern char last_output[256];
    TEST_ASSERT_TRUE(strstr(last_output, "ERROR") != NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_uart_send_result_valid);
    RUN_TEST(test_uart_send_result_invalid);
    return UNITY_END();
}