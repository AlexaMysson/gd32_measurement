#include "unity.h"
#include "calc.h"
#include <math.h>

#define FS 19200.0f
#define VREF_MV 3300.0f
#define OFFSET_V 1.65f
#define AMPLITUDE 1.5f

void setUp(void) {}
void tearDown(void) {}

static void generate_sine(uint16_t* samples, size_t n, float freq) {
    for (size_t i = 0; i < n; i++) {
        double t = i / FS;
        double v = AMPLITUDE * sin(2.0 * M_PI * freq * t);
        double adc_v = v + OFFSET_V;
        int code = (int)(adc_v * 4095.0 / (VREF_MV / 1000.0));
        if (code < 0) code = 0;
        if (code > 4095) code = 4095;
        samples[i] = (uint16_t)code;
    }
}

void test_frequency_50Hz(void) {
    uint16_t samples[2048];
    generate_sine(samples, 2048, 50.0f);
    CalcResult res;
    int status = calc_process_samples(samples, 2048, VREF_MV, &res);
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_INT(1, res.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 50.0f, res.frequency);
}

void test_frequency_40Hz(void) {
    uint16_t samples[2048];
    generate_sine(samples, 2048, 40.0f);
    CalcResult res;
    calc_process_samples(samples, 2048, VREF_MV, &res);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 40.0f, res.frequency);
}

void test_rms(void) {
    uint16_t samples[2048];
    generate_sine(samples, 2048, 50.0f);
    CalcResult res;
    calc_process_samples(samples, 2048, VREF_MV, &res);
    float expected_rms = AMPLITUDE / sqrtf(2.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, expected_rms, res.rms);
}

void test_first_harmonic_amplitude(void) {
    uint16_t samples[2048];
    generate_sine(samples, 2048, 50.0f);
    CalcResult res;
    calc_process_samples(samples, 2048, VREF_MV, &res);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, AMPLITUDE, res.first_harmonic_amp);
}

void test_df_dt_on_changing_frequency(void) {
    uint16_t samples[4096];
    for (size_t i = 0; i < 4096; i++) {
        double t = i / FS;
        double freq = 45.0 + 10.0 * t / (4096 / FS); 
        double v = AMPLITUDE * sin(2.0 * M_PI * freq * t);
        double adc_v = v + OFFSET_V;
        int code = (int)(adc_v * 4095.0 / (VREF_MV / 1000.0));
        if (code < 0) code = 0;
        if (code > 4095) code = 4095;
        samples[i] = (uint16_t)code;
    }
    CalcResult res;
    calc_process_samples(samples, 4096, VREF_MV, &res);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 46.9f, res.df_dt);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_frequency_50Hz);
    RUN_TEST(test_frequency_40Hz);
    RUN_TEST(test_rms);
    RUN_TEST(test_first_harmonic_amplitude);
    RUN_TEST(test_df_dt_on_changing_frequency);
    return UNITY_END();
}