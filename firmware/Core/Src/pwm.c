#include "pwm.h"
#include "stm32c031xx.h"

void pwm_init()
{
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;

    TIM14->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM Mode 1
    TIM14->CCER |= TIM_CCER_CC1E; // Capture/Compare 1 output enable
    TIM14->PSC = 24 - 1; // Increment counter each 24 ADC clock cyles (1 us)
    TIM14->ARR = 2000 - 1; // Reload counter each 2 ms
    TIM14->CCR1 = 1000;  // 50% duty cycle
    TIM14->EGR |= TIM_EGR_UG;
}

void pwm_enable()
{
    TIM14->CR1 |= TIM_CR1_CEN;
}

void pwm_disable()
{
    TIM14->CR1 &= ~TIM_CR1_CEN;
}
