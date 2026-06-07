#ifndef UART_OUT_H
#define UART_OUT_H

#include "calc.h"

void uart_init(void);
void uart_send_string(const char *str);
void uart_send_result(const CalcResult *result);

#endif
