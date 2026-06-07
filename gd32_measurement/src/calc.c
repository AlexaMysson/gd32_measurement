#define _USE_MATH_DEFINES
#include "calc.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FS 19200.0f
#define DC_OFFSET_V 1.65f
#define MAX_SAMPLES 4096   

static float adc_to_volts(uint16_t adc_value, float vref_mv) {
    float volts = (adc_value * vref_mv / 4095.0f) / 1000.0f;
    return volts - DC_OFFSET_V;
}

typedef struct {
    float time;
    int direction;
} ZeroCrossing;

static int find_zero_crossings(const float* signal, size_t n, ZeroCrossing* crossings, int max_crossings) {
    int count = 0;
    float dt = 1.0f / FS;
    for (size_t i = 1; i < n && count < max_crossings; i++) {
        float v_prev = signal[i - 1];
        float v_curr = signal[i];
        if ((v_prev <= 0.0f && v_curr > 0.0f) || (v_prev >= 0.0f && v_curr < 0.0f)) {
            float ratio = fabsf(v_prev) / (fabsf(v_prev) + fabsf(v_curr));
            float exact_time = (i - 1) * dt + ratio * dt;
            crossings[count].time = exact_time;
            crossings[count].direction = (v_curr > 0) ? 1 : -1;
            count++;
        }
    }
    return count;
}

static int extract_positive_crossings(const ZeroCrossing* crossings, int count, float* pos_times, int max_pos) {
    int pos_count = 0;
    for (int i = 0; i < count && pos_count < max_pos; i++) {
        if (crossings[i].direction == 1) {
            pos_times[pos_count++] = crossings[i].time;
        }
    }
    return pos_count;
}

static float calculate_frequency_exact(const float* pos_times, int pos_count, float* period_out) {
    if (pos_count < 4) return 0.0f;
    float interval = pos_times[3] - pos_times[0];
    float period = interval / 3.0f;
    if (period_out) *period_out = period;
    return 1.0f / period;
}

static float calculate_rms_on_interval(const float* signal, size_t n, float t_start, float t_end) {
    int idx_start = (int)(t_start * FS + 0.5f);
    int idx_end = (int)(t_end * FS + 0.5f);
    // Безопасные границы
    if (idx_start < 0) idx_start = 0;
    if (idx_start >= (int)n) idx_start = n - 1;
    if (idx_end <= idx_start) idx_end = idx_start + 1;
    if (idx_end > (int)n) idx_end = n;

    double sum_sq = 0.0;
    for (int i = idx_start; i < idx_end; i++) {
        sum_sq += (double)signal[i] * (double)signal[i];
    }
    return (float)sqrt(sum_sq / (idx_end - idx_start));
}

static float calculate_first_harmonic_on_interval(const float* signal, size_t n, float t_start, float t_end) {
    int idx_start = (int)(t_start * FS + 0.5f);
    int idx_end = (int)(t_end * FS + 0.5f);
    if (idx_start < 0) idx_start = 0;
    if (idx_start >= (int)n) idx_start = n - 1;
    if (idx_end <= idx_start) idx_end = idx_start + 1;
    if (idx_end > (int)n) idx_end = n;
    if (idx_end - idx_start < 4) return 0.0f;

    int N = idx_end - idx_start;
    double real = 0.0, imag = 0.0;
    for (int k = 0; k < N; k++) {
        double angle = 2.0 * M_PI * k / N;
        real += signal[idx_start + k] * cos(angle);
        imag -= signal[idx_start + k] * sin(angle);
    }
    return (2.0f / N) * (float)sqrt(real * real + imag * imag);
}

static float calculate_df_dt_exact(const float* pos_times, int pos_count) {
    if (pos_count < 7) return 0.0f;
    float period1 = (pos_times[3] - pos_times[0]) / 3.0f;
    float freq1 = 1.0f / period1;
    float period2 = (pos_times[pos_count - 1] - pos_times[pos_count - 4]) / 3.0f;
    float freq2 = 1.0f / period2;
    float delta_f = freq2 - freq1;
    float t1_mid = (pos_times[0] + pos_times[3]) / 2.0f;
    float t2_mid = (pos_times[pos_count - 4] + pos_times[pos_count - 1]) / 2.0f;
    float dt = t2_mid - t1_mid;
    if (dt > 0.0f) return delta_f / dt;
    return 0.0f;
}

int calc_process_samples(const uint16_t* samples, size_t num, float vref_mv, CalcResult* result) {
    memset(result, 0, sizeof(CalcResult));
    if (num < 100 || num > MAX_SAMPLES) return -1; 

    static float volts[MAX_SAMPLES];
    for (size_t i = 0; i < num; i++) {
        volts[i] = adc_to_volts(samples[i], vref_mv);
    }

    ZeroCrossing crossings[50];
    int num_cross = find_zero_crossings(volts, num, crossings, 50);
    if (num_cross < 8) return -1;

    float pos_times[20];
    int pos_count = extract_positive_crossings(crossings, num_cross, pos_times, 20);
    if (pos_count < 4) return -1;

    float period;
    result->frequency = calculate_frequency_exact(pos_times, pos_count, &period);
    if (result->frequency <= 0.0f) return -1;

    if (pos_count >= 4) {
        result->rms = calculate_rms_on_interval(volts, num, pos_times[0], pos_times[3]);
    }
    else {
        result->rms = 0.0f;
    }

    if (pos_count >= 4) {
        result->first_harmonic_amp = calculate_first_harmonic_on_interval(volts, num, pos_times[0], pos_times[3]);
    }
    else {
        result->first_harmonic_amp = 0.0f;
    }

    result->df_dt = calculate_df_dt_exact(pos_times, pos_count);

    result->valid = 1;
    return 0;
}