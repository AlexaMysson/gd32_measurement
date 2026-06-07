#ifndef CALC_H
#define CALC_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    float rms;                  
    float frequency;            
    float df_dt;                
    float first_harmonic_amp;   
    int valid;                  
} CalcResult;

int calc_process_samples(const uint16_t* samples, size_t num, float vref_mv, CalcResult* result);

#endif