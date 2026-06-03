#include "assert.h"
#include "stm32c031xx.h"
#include "tim.h"

static void tim_set_prescaler(uint16_t prescaler)
{
    TIM1->PSC = prescaler;
}

static void tim_set_arv(uint16_t arv)
{
    TIM1->ARR = arv;
}

void tim_init()
{
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;

    TIM1->CR2 |= TIM_CR2_MMS2_0 | TIM_CR2_MMS2_1; // Set compare pulse as trigger output (TRGO2)
    TIM1->SMCR |= TIM_SMCR_MSM; // Enable master mode

    TIM1->DIER |= TIM_DIER_UIE; // Enable update interrupt

    // Note In this configuration, each time counter reaches 0, signal is generated on TRGO2 until counter reaches 1.
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // "Select PWM mode 1 as output compare 1 mode. In up-counting mode, channel 1 is active as long as TIMx_CNT<TIMx_CCR1, else inactive."
    TIM1->CCR1 = 1; // Capture/Compare 1 value
}

void tim_enable()
{
    TIM1->CR1 |= TIM_CR1_CEN;
}

void tim_disable()
{
    TIM1->CR1 &= ~TIM_CR1_CEN;
}

void tim_configure_for_sampling(uint32_t ext_trig_interval)
{
    uint16_t prescaler = 1;
    uint16_t arv = (ext_trig_interval >> 1) - 1;

    tim_set_prescaler(prescaler);
    tim_set_arv(arv);

    TIM1->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF); // Clear UIF and CC1I flags
    TIM1->CNT &= ~(TIM_CNT_CNT); // Clear CNT register
}

void tim_cleanup_after_sampling()
{
    TIM1->RCR &= ~(TIM_RCR_REP); // Clear RCR preload register
    TIM1->EGR |= TIM_EGR_UG; // Clear RCR shadow register by generating UEV
}

uint16_t tim_get_cycles_to_uev()
{
    uint32_t cycles = (TIM1->PSC * (TIM1->ARR - TIM1->CNT + 1));

    return cycles;
}
