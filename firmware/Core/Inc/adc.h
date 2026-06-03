#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdint.h>
#include "stm32c0xx.h"
#include "osc_protocol.h"

#define ADC_BUFF_SIZE_EXP (10U)
#define ADC_BUFF_SIZE (1U << ADC_BUFF_SIZE_EXP)
#define ADC_BUFF_SIZE_MSK (ADC_BUFF_SIZE - 1U)

typedef enum adc_resolution
{
	ADC_RESOLUTION_12BIT = 0U,
	ADC_RESOLUTION_10BIT = ADC_CFGR1_RES_0,
	ADC_RESOLUTION_8BIT = ADC_CFGR1_RES_1,
	ADC_RESOLUTION_6BIT = ADC_CFGR1_RES_0 | ADC_CFGR1_RES_1,
} adc_resolution;

typedef enum adc_sampling_time {
    ADC_SAMPLING_TIME_1_5 = 0U,
    ADC_SAMPLING_TIME_3_5 = 1U
} adc_sampling_time;

typedef struct {
    uint32_t ext_trig_interval; // In ADC clock cycles
    uint16_t scan_count;
} adc_acq_params;

void adc_init();

void adc_configure_for_sampling(osc_echannel channels, adc_sampling_time sampling_time, adc_resolution resolution);

void adc_cleanup_after_sampling();

uint16_t* adc_get_buff();

/*
 * @brief This function calculates optimal acquisition parameters.
 */
adc_acq_params adc_calculate_acq_params(uint32_t acquisition_time_us, uint16_t max_sample_count, uint8_t channel_count);

#endif /* INC_ADC_H_ */
