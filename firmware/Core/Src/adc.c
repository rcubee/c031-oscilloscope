#include "adc.h"

#define ADC_CLK_MHZ 24

static uint16_t adc_buff[ADC_BUFF_SIZE];

static void adc_voltage_regulator_enable()
{
	ADC1->CR |= ADC_CR_ADVREGEN;

	// Note: Wait for 20us. TODO: Replace with SysTick / clock-speed dependent delay.
	for (uint32_t i = 0; i < 1000; ++i) {
	    __NOP();
	}
}

static void adc_enable()
{
	ADC1->ISR |= ADC_ISR_ADRDY;

	ADC1->CR |= ADC_CR_ADEN;

	while (!(ADC1->ISR & ADC_ISR_ADRDY))
	    ;
}

static void adc_disable()
{
	if (ADC1->CR & ADC_CR_ADSTART) {
		ADC1->CR |= ADC_CR_ADSTP;

		while (ADC1->CR & ADC_CR_ADSTP)
		    ;
	}

	if (!(ADC1->CR & ADC_CR_ADEN)) {
		return;
	}

	ADC1->CR |= ADC_CR_ADDIS;

	while (ADC1->CR & ADC_CR_ADEN)
	    ;

	ADC1->ISR |= ADC_ISR_ADRDY;
}

// Note: RM0490 Reference manual, 16.4.3 Calibration (ADCAL)
static void adc_calibrate()
{
	uint32_t calibration_factor = 0;
	volatile uint32_t calibration_factor_sum = 0;

	adc_disable();

	ADC1->CFGR1 &= ~(ADC_CFGR1_AUTOFF | ADC_CFGR1_DMAEN);

	for (unsigned int i = 0; i < 8; ++i) {
		ADC1->CR |= ADC_CR_ADCAL;

		while (ADC1->CR & ADC_CR_ADCAL)
		    ;

		calibration_factor_sum += (ADC1->CALFACT & ADC_CALFACT_CALFACT) + 1;
	}

	calibration_factor = (calibration_factor_sum + 7) / 8;

	if (calibration_factor > 0x7F) {
		calibration_factor = 0x7F;
	}

	// Note: The calibration factor can be written if the ADC is enabled but not converting (ADEN = 1 and ADSTART = 0).
	adc_enable();
	ADC1->CALFACT = calibration_factor;
	adc_disable();
}

static void adc_set_resolution(adc_resolution resolution)
{
	ADC1->CFGR1 = (ADC1->CFGR1 & ~ADC_CFGR1_RES) | resolution;
}

// Note: RM0490 Reference manual, 16.12.1 ADC interrupt and status register (ADC_ISR)
static void adc_wait_for_ccrdy()
{
    while (!(ADC1->ISR & ADC_ISR_CCRDY))
        ;

    ADC1->ISR = ADC_ISR_CCRDY;
}

static void adc_select_channels(osc_echannel channels)
{
	while (ADC1->CR & ADC_CR_ADSTART)
	    ;

	uint32_t chsel = 0;
	chsel |= channels & OSC_ECHANNEL_1 ? ADC_CHSELR_CHSEL0 : 0;
	chsel |= channels & OSC_ECHANNEL_2 ? ADC_CHSELR_CHSEL1 : 0;

	ADC1->CHSELR = (ADC1->CHSELR & (~ADC_CHSELR_CHSEL)) | chsel;

	adc_wait_for_ccrdy();

	// Note: Sequencer not fully configurable, forward scan
	ADC1->CFGR1 &= ~(ADC_CFGR1_CHSELRMOD | ADC_CFGR1_SCANDIR);

	adc_wait_for_ccrdy();
}

static void adc_set_sampling_time(adc_sampling_time sampling_time)
{
	while (ADC1->CR & ADC_CR_ADSTART)
	    ;

	ADC1->SMPR = (ADC1->SMPR & ~ADC_SMPR_SMP1) | sampling_time;

	// Set channels to use the setting of SMP1[2:0] register
	ADC1->SMPR &= ~ADC_SMPR_SMPSEL0;
	ADC1->SMPR &= ~ADC_SMPR_SMPSEL1;
}

/*
 * @brief Returns the t_smpl in ADC clock cycles for given sampling time, rounded down.
 */
static uint8_t adc_sampling_time_to_t_smpl(adc_sampling_time sampling_time)
{
    switch (sampling_time)
    {
    case ADC_SAMPLING_TIME_1_5: return 1;
    default: return 3;
    }
}

/*
 * @brief Returns the t_sar in ADC clock cycles for given resolution, rounded down.
 */
static uint8_t adc_resolution_to_t_sar(adc_resolution resolution)
{
    switch (resolution)
    {
    case ADC_RESOLUTION_6BIT: return 6;
    case ADC_RESOLUTION_8BIT: return 8;
    case ADC_RESOLUTION_10BIT: return 10;
    default: return 12;
    }
}

/*
 * @brief Calculate t_conv in ADC clock cycles.
 */
static uint8_t adc_calculate_t_conv(adc_sampling_time sampling_time, adc_resolution resolution)
{
    // Note: Conversion time is calculated according to the formula: t_conv = t_smpl + t_sar (RM0490, 16.4.13 Timings).

    uint8_t t_smpl = adc_sampling_time_to_t_smpl(sampling_time);
    uint8_t t_sar = adc_resolution_to_t_sar(resolution);
    uint8_t t_conv = t_smpl + t_sar + 1; // Note: Add 1 to compensate for rounding down of t_smpl and t_sar.

    return t_conv;
}

/*
 * @brief Calculates minimum sequence conversion interval in ADC clock cycles.
 *
 * @warning This function assumes that
 * 1. CKMODE = 00
 * 2. single trigger starts the entire sequence (CONT = 0 and DISC = 0).
 */
static uint32_t adc_calculate_min_ext_trig_interval(adc_sampling_time sampling_time, adc_resolution resolution, uint8_t channel_count)
{
    uint8_t t_latr_adc_cycles = 3; // DS13867 Rev 4, 5.3.15 Analog-to-digital converter characteristics, Table 54. ADC characteristics

    // Note: Add 1 ADC clock cycle to ensure that external trigger doesn't occur exactly at end of sequence.
    uint32_t min_ext_trig_interval = t_latr_adc_cycles + adc_calculate_t_conv(sampling_time, resolution) * channel_count + 1; // In ADC clock cycles

    return min_ext_trig_interval;
}

void adc_init()
{
	RCC->APBENR2 |= RCC_APBENR2_ADCEN;

	// Note: Select SYSCLK as internal clock source for ADC
	RCC->CCIPR &= ~RCC_CCIPR_ADCSEL;
	ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;

	adc_voltage_regulator_enable();

	adc_calibrate();

	ADC1->CFGR1 =
	    ADC_CFGR1_DMAEN // Enable DMA
	    | ADC_CFGR1_DMACFG // DMA circular mode
	    | ADC_CFGR1_EXTEN_0; // Hardware trigger detection on the rising edge
}

void adc_configure_for_sampling(osc_echannel channels, adc_sampling_time sampling_time, adc_resolution resolution)
{
    adc_select_channels(channels);
    adc_set_sampling_time(sampling_time);
    adc_set_resolution(resolution);

    adc_enable();

    // Note: External trigger becomes effective when ADSTART = 1
    ADC1->CR |= ADC_CR_ADSTART;
}

void adc_cleanup_after_sampling()
{
    adc_disable();
}

uint16_t* adc_get_buff()
{
    return adc_buff;
}

adc_acq_params adc_calculate_acq_params(uint32_t acquisition_time_us, uint16_t max_sample_count, uint8_t channel_count)
{
    assert(channel_count > 0 && channel_count < 3);

    uint32_t acquisition_time = acquisition_time_us * ADC_CLK_MHZ; // In ADC clock cycles

    // Note: Interval at which timer's external trigger should start sequence of conversions in ADC.
    uint32_t ext_trig_interval = ((acquisition_time + max_sample_count - 1) / max_sample_count) * channel_count; // In ADC clock cycles

    uint32_t min_ext_trig_interval = adc_calculate_min_ext_trig_interval(ADC_SAMPLING_TIME_3_5, ADC_RESOLUTION_10BIT, channel_count);
    if (ext_trig_interval < min_ext_trig_interval) {
        ext_trig_interval = min_ext_trig_interval;
    }

    assert(acquisition_time >= ext_trig_interval);

    uint16_t scan_count = (acquisition_time / ext_trig_interval) & ~0b1; // Even number of samples

    adc_acq_params acquisition_parameters = {
        .ext_trig_interval = ext_trig_interval,
        // .scan_count = samples_per_channel
        .scan_count = scan_count
    };

    return acquisition_parameters;
}
