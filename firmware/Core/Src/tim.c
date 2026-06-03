#include "assert.h"
#include "stm32c031xx.h"
#include "tim.h"

// Note: Assuming that CK_INT is 24MHz
// The timer's counter is blocked while the auto-reload value is null.
// The goal is to collect up to 100 samples for each timediv.
static void horizontal_scale_to_prescaler_and_arv(osc_horizontal_scale horizontal_scale, uint16_t* prescaler, uint16_t* arv)
{
    switch (horizontal_scale)
    {
        case OSC_HORIZONTAL_SCALE_10us:
        case OSC_HORIZONTAL_SCALE_20us:
        case OSC_HORIZONTAL_SCALE_50us:
        case OSC_HORIZONTAL_SCALE_100us:
        {
            // Collect sample each 1us
            *prescaler = 12 - 1;
            *arv = 2 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_200us:
        {
            // Collect sample each 2us
            *prescaler = 24 - 1;
            *arv = 2 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_500us:
        {
            // Collect sample each 5us
            *prescaler = 24 - 1;
            *arv = 5 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_1ms:
        {
            // Collect sample each 10us
            *prescaler = 24 - 1;
            *arv = 10 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_2ms:
        {
            // Collect sample each 20us
            *prescaler = 24 - 1;
            *arv = 20 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_5ms:
        {
            // Collect sample each 50us
            *prescaler = 24 - 1;
            *arv = 50 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_10ms:
        {
            // Collect sample each 100us
            *prescaler = 24 - 1;
            *arv = 100 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_20ms:
        {
            // Collect sample each 200us
            *prescaler = 24 - 1;
            *arv = 200 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_50ms:
        {
            // Collect sample each 500us
            *prescaler = 24 - 1;
            *arv = 500 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_100ms:
        {
            // Collect sample each 1ms
            *prescaler = 24 - 1;
            *arv = 1000 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_200ms:
        {
            // Collect sample each 2ms
            *prescaler = 24 - 1;
            *arv = 2000 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_500ms:
        {
            // Collect sample each 5ms
            *prescaler = 24 - 1;
            *arv = 5000 - 1;
            break;
        };

        case OSC_HORIZONTAL_SCALE_1s:
        {
            // Collect sample each 10ms
            *prescaler = 24000 - 1;
            *arv = 10 - 1;
            break;
        };

        default:
            assert(0);
    }
}

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

    /* Note:
     * In this configuration, each time counter reaches 0, signal is generated on TRGO2
     * until counter reaches 1.
     */
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
