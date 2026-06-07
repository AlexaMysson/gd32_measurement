#include "uart_out.h"

#ifdef TEST

#include <string.h>
static char last_output[256];
static int output_len = 0;
void uart_send_string(const char* str) {
    strcpy(last_output, str);
    output_len = strlen(str);
}
#else

#ifdef PLATFORM_HOST
#include <stdio.h>
void uart_init(void) { /* ничего */ }
void uart_send_string(const char* str) { printf("%s", str); }
#else
#include "gd32f4xx.h"
void uart_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}
void uart_send_string(const char* str) {
    while (*str) {
        while (!usart_flag_get(USART0, USART_FLAG_TBE));
        usart_data_transmit(USART0, (uint8_t)*str++);
    }
}
#endif 
#endif 

void uart_send_result(const CalcResult* result) {
    if (!result->valid) {
        uart_send_string("ERROR: Invalid measurement\n");
        return;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "RMS=%.3f V, Freq=%.2f Hz, dF/dt=%.2f Hz/s, 1stHarm=%.3f V\n",
        result->rms, result->frequency, result->df_dt, result->first_harmonic_amp);
    uart_send_string(buf);
}