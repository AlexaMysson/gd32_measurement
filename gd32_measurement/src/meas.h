#ifndef MEAS_H
#define MEAS_H

#include <stdint.h>
#include <stddef.h>

void meas_init(void);

void meas_start(void);

const uint16_t* meas_get_current_buffer(size_t* len);

void meas_buffer_processed(void);

void meas_set_signal(float frequency_hz, float amplitude_v);

#endif