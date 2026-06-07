#include "meas.h"

#ifdef PLATFORM_HOST

#include <math.h>
#include <stdio.h>
#define BUFFER_SIZE 2048
#define FS 19200.0f
#define VREF_MV 3300.0f
#define DC_OFFSET_V 1.65f

static uint16_t buffer[BUFFER_SIZE];
static size_t sample_index = 0;
static int buffer_ready = 0;
static float signal_freq = 50.0f;
static float signal_amp = 1.5f;

void meas_init(void) {
    printf("Measurement simulation initialized\n");
}
void meas_start(void) {
    sample_index = 0;
    buffer_ready = 0;
}
void meas_set_signal(float freq, float amp) {
    signal_freq = freq;
    signal_amp = amp;
}
const uint16_t* meas_get_current_buffer(size_t* len) {
    if (!buffer_ready) {
        for (size_t i = 0; i < BUFFER_SIZE; i++) {
            double t = (sample_index + i) / FS;
            double v = signal_amp * sin(2 * M_PI * signal_freq * t);
            double adc_v = v + DC_OFFSET_V;
            int code = (int)(adc_v * 4095 / (VREF_MV / 1000));
            if (code < 0) code = 0; if (code > 4095) code = 4095;
            buffer[i] = (uint16_t)code;
        }
        sample_index += BUFFER_SIZE;
        buffer_ready = 1;
    }
    *len = BUFFER_SIZE;
    return buffer;
}
void meas_buffer_processed(void) { buffer_ready = 0; }

#else 

#include "gd32f4xx.h"
#define ADC_BUFFER_SIZE 2048

static uint16_t adc_buffer[ADC_BUFFER_SIZE];
static volatile uint8_t half_done = 0, full_done = 0;

void meas_init(void) {

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_ADC);
    rcu_periph_clock_enable(RCU_DMA);
    rcu_periph_clock_enable(RCU_TIMER0);


    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_0);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);

    
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, 1);
    adc_regular_channel_config(ADC0, 0, ADC_CH_0, ADC_SAMPLETIME_15);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REG_TIMER0_TRGO);
    adc_external_trigger_edge_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_EDGE_RISING);
    adc_dma_mode_enable(ADC0);
    adc_enable(ADC0);
    adc_calibration_enable(ADC0);

  
    dma_parameter_struct dma_init_struct;
    dma_deinit(DMA0, DMA_CH0);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)adc_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.number = ADC_BUFFER_SIZE;
    dma_init_struct.periph_addr = (uint32_t)&ADC_RDATA(ADC0);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_init_struct);
    dma_circulation_enable(DMA0, DMA_CH0);
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_HTF);
    dma_channel_enable(DMA0, DMA_CH0);

    timer_parameter_struct timer_init;
    timer_oc_parameter_struct oc_init;
    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_init);
    timer_init.prescaler = 168 - 1;  
    timer_init.counter_direction = TIMER_COUNTER_UP;
    timer_init.period = 52 - 1;      
    timer_init.clock_division = TIMER_CKDIV_DIV1;
    timer_init(TIMER0, &timer_init);
    timer_oc_struct_para_init(&oc_init);
    oc_init.output_state = TIMER_CCX_DISABLE;
    timer_oc_config(TIMER0, TIMER_OC_0, &oc_init);
    timer_primary_output_config(TIMER0, ENABLE);
    timer_master_slave_config(TIMER0, TIMER_MS_MASTER);
    timer_master_output_trigger_source_config(TIMER0, TIMER_MOS_UPDATE);
    timer_enable(TIMER0);

    nvic_irq_enable(DMA0_Channel0_IRQn, 1, 0);
}

void meas_start(void) {
    half_done = 0;
    full_done = 0;
    dma_channel_enable(DMA0, DMA_CH0);
}

const uint16_t* meas_get_current_buffer(size_t* len) {
    if (half_done) {
        *len = ADC_BUFFER_SIZE / 2;
        half_done = 0;
        return adc_buffer;
    }
    if (full_done) {
        *len = ADC_BUFFER_SIZE / 2;
        full_done = 0;
        return adc_buffer + ADC_BUFFER_SIZE / 2;
    }
    return NULL;
}

void meas_buffer_processed(void) {
}

void DMA0_Channel0_IRQHandler(void) {
    if (dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_HTF)) {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_HTF);
        half_done = 1;
    }
    if (dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FTF)) {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FTF);
        full_done = 1;
    }
}

#endif 