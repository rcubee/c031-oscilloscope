#include "adc.h"

static uint16_t adc_buff[ADC_BUFF_SIZE];

void adc_voltage_regulator_enable()
{
	ADC1->CR |= ADC_CR_ADVREGEN;

	// Note: Wait for 20us
	for (uint32_t i = 0; i < 1000; ++i)
		;
}

void adc_enable()
{
	ADC1->ISR |= ADC_ISR_ADRDY;

	ADC1->CR |= ADC_CR_ADEN;

	while (!(ADC1->ISR & ADC_ISR_ADRDY))
		;
}

void adc_disable()
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

void adc_voltage_regulator_disable()
{
	adc_disable();

	ADC1->CR &= ~ADC_CR_ADVREGEN;
}

// Note: RM0490 Reference manual, 16.4.3 Calibration (ADCAL)
void adc_callibrate()
{
	uint32_t callibration_factor = 0;
	volatile uint32_t callibration_factor_sum = 0;

	adc_disable();

	ADC1->CFGR1 &= ~(ADC_CFGR1_AUTOFF | ADC_CFGR1_DMAEN);

	for (unsigned int i = 0; i < 8; ++i) {
		ADC1->CR |= ADC_CR_ADCAL;

		while (ADC1->CR & ADC_CR_ADCAL)
			;

		callibration_factor_sum += (ADC1->CALFACT & ADC_CALFACT_CALFACT) + 1;
	}

	callibration_factor = (callibration_factor_sum + 7) / 8;

	if (callibration_factor > 0x7F) {
		callibration_factor = 0x7F;
	}

	ADC1->CALFACT = callibration_factor;
}

void adc_set_resolution(adc_resolution resolution)
{
	ADC1->CFGR1 = (ADC1->CFGR1 & ~ADC_CFGR1_RES) | resolution;
}

void adc_set_conversion_mode()
{
	ADC1->CFGR1 &= ~ADC_CFGR1_CONT; // Single conversion mode
}

void adc_select_channels()
{
	while (ADC1->CR & ADC_CR_ADSTART)
		;

	// Note: Select channel 0
	ADC1->CHSELR |= ADC_CHSELR_CHSEL0;

	// Note: Sequencer not fully configurable, forward scan
	ADC1->CFGR1 &= ~(ADC_CFGR1_CHSELRMOD | ADC_CFGR1_SCANDIR);
}

void adc_set_sampling_time(adc_sampling_time sampling_time)
{
	while (ADC1->CR & ADC_CR_ADSTART)
		;

	ADC1->SMPR = (ADC1->SMPR & ~ADC_SMPR_SMP1) | sampling_time;

	ADC1->SMPR &= ~ADC_SMPR_SMPSEL0; // Set channel 0 to use the setting of SMP1[2:0] register
}

// Note: With 10 bit resolution, it takes 12 ADC_CLK cycles (with minimum t_sample of 1.5 cycles)
// Since ADC_CLK is 24MHz, 12 cycles take 0.5uS
// (Theoretical max (when ADC_CLK is 35MHz and resolution is 12 bits) is 0.4uS)
// If we assume that 10 samples for a period of a signal are good enough (thinking about sinusoidal signal),
// with time of 0.5uS for each sample, the theoretical maximum freq we can measure is 200kHz
void adc_start_conversions()
{
	ADC1->CR |= ADC_CR_ADSTART;

	while (!(ADC1->ISR & ADC_ISR_EOS))
		;
}

void adc_enable_dma()
{
    ADC1->CFGR1 |=
        ADC_CFGR1_DMAEN
        | ADC_CFGR1_DMACFG;
}

void adc_init()
{
	RCC->APBENR2 |= RCC_APBENR2_ADCEN;

	// Note: Select SYSCLK as internal clock source for ADC
	RCC->CCIPR &= ~RCC_CCIPR_ADCSEL;
	ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;

	adc_voltage_regulator_enable();

	adc_callibrate();

	ADC1->CFGR1 =
	    ADC_CFGR1_DMAEN // Enable DMA
	    | ADC_CFGR1_DMACFG // DMA circular mode
	    | ADC_CFGR1_EXTEN_0; // Hardware trigger detection on the rising edge

	ADC1->CHSELR |= ADC_CHSELR_CHSEL0; // Select channel 0
}


void adc_configure_for_sampling(adc_resolution resolution, adc_sampling_time sampling_time)
{
    adc_set_resolution(resolution);
    adc_set_sampling_time(sampling_time);

    adc_enable();

    // Note: External trigger is effective when ADSTART = 1
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
