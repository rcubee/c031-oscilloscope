#include "adc.h"
#include <assert.h>
#include <string.h>
#include "debug.h"
#include "dma.h"
#include "gpio.h"
#include "osc.h"
#include "osc_protocol.h"
#include "pwm.h"
#include "tim.h"
#include "stm32c031xx.h"
#include "systick.h"
#include "usart.h"

static volatile osc g_osc;

static volatile osc *osc_get()
{
    return &g_osc;
}

static volatile osc_trigger *osc_get_trigger()
{
    return &g_osc.trigger;
}

static osc_config osc_get_default_config()
{
    osc_config config = {
        .adc_resolution = ADC_RESOLUTION_10BIT,
        .channel1 = 0,
        .horizontal_scale = OSC_HORIZONTAL_SCALE_200us,
        .sample_count = 1000,
        .pre_trigger_sample_count = 250
    };

    return config;
}

static void osc_cleanup_after_sampling()
{
    adc_cleanup_after_sampling();
    dma_cleanup_after_sampling();
    tim_cleanup_after_sampling();
}

static uint32_t osc_trigger_get_total_collected_samples(osc_trigger *trigger)
{
    return
        (trigger->transfer_complete_count * ADC_BUFF_SIZE)
        + (ADC_BUFF_SIZE - DMA1_Channel3->CNDTR);
}

static void osc_transmit_message(const char* message)
{
    uint8_t buff[128];

    osc_frame_message *frame = osc_build_frame_message(buff, sizeof(buff), message);

    usart_transmit_dma((uint8_t*)frame, sizeof(osc_frame_message) + frame->length);
}

static void osc_transmit_samples(osc* oscilloscope)
{
    if (oscilloscope->state != OSC_STATE_SAMPLING) {
        // Note: Sampling was disabled.
        return;
    }

    osc_frame_header frame_header = {
        .sync = OSC_FRAME_SYNC,
        .type = OSC_FRAME_TYPE_SAMPLES
    };

    if (osc_trigger_get_total_collected_samples(osc_get_trigger()) < 1000) {
        assert(0);
    }

    usart_transmit_dma((uint8_t*)&frame_header, sizeof(osc_frame_header));

    uint16_t *adc_buff = adc_get_buff();

    usart_transmit_dma((uint8_t*)&(oscilloscope->config.sample_count), sizeof(uint16_t));

    for (uint16_t i = 0; i < oscilloscope->config.sample_count; ++i) {
        uint16_t sample_index = (oscilloscope->trigger.sample_index + i - oscilloscope->config.pre_trigger_sample_count) & ADC_BUFF_SIZE_MSK;
        uint16_t sample = adc_buff[sample_index];

        usart_transmit_dma((uint8_t*)&sample, sizeof(uint16_t));
    }

    oscilloscope->trigger.estate = OSC_TRIGGER_ESTATE_IDLE;

    // LOG("Transmitted all samples");
}

/* Note:
 * This ISR is responsible for ensuring that no overrun of ADC
 * data occurs.
 *
 *  When DMA is enabled, tc, hf and te interrupts can't be disabled.
 */
void DMA1_Channel2_3_IRQHandler()
{
    if (!(DMA1->ISR & DMA_ISR_GIF3)) {
        return;
    }

    if (DMA1->ISR & DMA_ISR_TEIF3) {
        assert(0);
    }

    osc* oscilloscope = osc_get();

    if (DMA1->ISR & DMA_ISR_TCIF3) {
        oscilloscope->trigger.transfer_complete_count += 1;
    }

    if (oscilloscope->trigger.estate >= OSC_TRIGGER_ESTATE_COLLECT_POST_TRIGGER_SAMPLES) {
        goto DMA1_Channel2_3_IRQHandler_cleanup; // Note: Timer is responsible for ensuring that no overrun occurs
    }

    // bool processing_second_half = oscilloscope->trigger.sample_index & (1 << (ADC_BUFF_SIZE_EXP - 1));
    bool processing_first_half = oscilloscope->trigger.sample_index < (ADC_BUFF_SIZE >> 1);
    bool transferring_to_first_half = DMA1->ISR & DMA_ISR_TCIF3;
    bool transferring_to_second_half = DMA1->ISR & DMA_ISR_HTIF3;

    if ((transferring_to_first_half && processing_first_half) || (transferring_to_second_half && !processing_first_half)) {
        // Note: At this point, there is no guarantee that transferred data will not corrupt not-yet-processed data.
        tim_disable();

        osc_cleanup_after_sampling();

        oscilloscope->state = OSC_STATE_IDLE;
    }

DMA1_Channel2_3_IRQHandler_cleanup:
    DMA1->IFCR |= DMA_IFCR_CGIF3;
}

/* Note:
 * This ISR executed after all post-trigger samples have been
 * collected. It is responsible for stopping or restarting sampling.
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler()
{
    if (!(TIM1->SR & TIM_SR_UIF)) {
        // Note: Only update event interrupt should be enabled.
        assert(0);
    }

    tim_disable();

    /* Cleanup peripherals after sampling */
    osc_cleanup_after_sampling();

    osc_trigger* trigger = osc_get_trigger();
    trigger->estate = OSC_TRIGGER_ESTATE_TRANSMIT_SAMPLES;

    /* Clear & disable the timer interrupt */
    TIM1->SR &= ~TIM_SR_UIF;

    NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

static void osc_trigger_reset(osc_trigger *trigger)
{
    trigger->estate = OSC_TRIGGER_ESTATE_IDLE;
    trigger->sample_index = 0;
    trigger->transfer_complete_count = 0;

    // TODO: Move
    trigger->type.algo.data.threshold.fell_below_low_threshold = false;
}

void osc_init()
{
#ifdef DEBUG
    debug_init();
#endif // DEBUG

    osc* oscilloscope = osc_get();
    *oscilloscope = (osc) {
        .state = OSC_STATE_IDLE,

        .config = osc_get_default_config(),

        .trigger = (osc_trigger) {
            .mode = OSC_TRIGGER_MODE_AUTO,
            .type = (osc_trigger_type) {
                .etype = OSC_TRIGGER_ETYPE_THRESHOLD,
                .algo = (osc_trigger_algo) {
                    .fn = osc_trigger_algo_fn_threshold,
                    .data.threshold = (osc_trigger_algo_data_threshold) {
                        .threshold_low = (1 << (ADC_BUFF_SIZE_EXP - 2)) - 1,
                        .threshold_high = (1 << (ADC_BUFF_SIZE_EXP - 1)) - 1,
                        .fell_below_low_threshold = false
                    }
                }
            },

            .estate = OSC_TRIGGER_ESTATE_IDLE,
            .sample_index = 0,
            .transfer_complete_count = 0
        }
    };

    adc_init();
    dma_init();
    gpio_init();
    pwm_init();
    systick_init();
    tim_init();
    usart_init();

    LOG("Build date: %s", __TIMESTAMP__);
}

static void osc_handle_protocol(osc* oscilloscope)
{
    uint8_t *buff;
    uint8_t buff_len;

    usart_receive_dma(&buff, &buff_len);

    osc_frame_type type;
    size_t index = osc_frame_parse_header(buff, buff_len, &type);
    switch (type) {
    case OSC_FRAME_TYPE_STOP_COLLECTING_SAMPLES:
    {
        oscilloscope->state = OSC_STATE_IDLE;

        pwm_disable();

        break;
    }

    case OSC_FRAME_TYPE_START_COLLECTING_SAMPLES:
    {
        oscilloscope->state = OSC_STATE_SAMPLING;

        pwm_enable();

        break;
    }

    case OSC_FRAME_TYPE_GET_CONFIG:
    {
        osc_frame_config frame;
        frame.header.sync = OSC_FRAME_SYNC;
        frame.header.type = OSC_FRAME_TYPE_CONFIG;
        frame.ch1 = oscilloscope->state == OSC_STATE_SAMPLING;
        frame.ch2 = 0;
        frame.horizontal_scale = oscilloscope->config.horizontal_scale;

        usart_transmit_dma((uint8_t*)&frame, sizeof(osc_frame_config));

        break;
    }

    case OSC_FRAME_TYPE_CONFIG:
    {
        osc_frame_config* frame = (osc_frame_config*)(buff + index);

        oscilloscope->config.horizontal_scale = frame->horizontal_scale;

        break;
    }

    default: break;
    }
}

void osc_sample();

void osc_run()
{
    osc* oscilloscope = osc_get();

    while (1) {
        osc_handle_protocol(oscilloscope);

        osc_sample();
    }
}

void osc_wait_for_pre_trigger_samples(osc* oscilloscope);
void osc_wait_for_trigger(osc* oscilloscope, uint8_t timeout_ms);
void osc_wait_for_post_trigger_samples(osc* oscilloscope);

void osc_sample()
{
    osc* oscilloscope = osc_get();
    if (oscilloscope->state != OSC_STATE_SAMPLING) {
        return;
    }

    osc_trigger_reset(&oscilloscope->trigger);

    tim_configure_for_sampling(oscilloscope->config.horizontal_scale);
    adc_configure_for_sampling(oscilloscope->config.adc_resolution, ADC_SAMPLING_TIME_3_5);
    dma_configure_for_sampling();

    /* Note: When sampling starts, software must process collected samples
     * (update the sample_index) fast enough, so that ADC overrun doesn't occur.
     */
    tim_enable();

    osc_wait_for_pre_trigger_samples(oscilloscope);
    osc_wait_for_trigger(oscilloscope, 100);
    osc_wait_for_post_trigger_samples(oscilloscope);
    osc_transmit_samples(oscilloscope);
}

void osc_wait_for_pre_trigger_samples(osc* oscilloscope)
{
    osc_trigger* trigger = &oscilloscope->trigger;

    trigger->estate = OSC_TRIGGER_ESTATE_COLLECT_PRE_TRIGGER_SAMPLES;

    while (osc_trigger_get_total_collected_samples(trigger) < oscilloscope->config.pre_trigger_sample_count) {
        uint16_t sample_count = ((ADC_BUFF_SIZE - DMA1_Channel3->CNDTR) - trigger->sample_index + ADC_BUFF_SIZE) & ADC_BUFF_SIZE_MSK;

        trigger->sample_index += sample_count;
    }

    trigger->sample_index = oscilloscope->config.pre_trigger_sample_count;

    // LOG("Captured %u pre-trigger samples",
    //     oscilloscope->config.pre_trigger_sample_count);
}

void osc_wait_for_trigger(osc* oscilloscope, uint8_t timeout_ms)
{
    osc_trigger* trigger = &oscilloscope->trigger;
    const uint16_t* adc_buff = adc_get_buff();

    trigger->estate = OSC_TRIGGER_ESTATE_WAIT_FOR_TRIGGER;
    uint32_t millis_start = systick_get_millis();

    while (trigger->estate == OSC_TRIGGER_ESTATE_WAIT_FOR_TRIGGER) {
        if (oscilloscope->state != OSC_STATE_SAMPLING) {
            break; // TODO: Disable sampling
        }

        if ((trigger->mode == OSC_TRIGGER_MODE_AUTO)
            && (systick_get_millis() - millis_start) >= timeout_ms) {
            // LOG("Trigger not found. Displaying gathered samples due to AUTO mode.");
            trigger->estate = OSC_TRIGGER_ESTATE_COLLECT_POST_TRIGGER_SAMPLES;
            break;
        }

        uint16_t sample_count = ((ADC_BUFF_SIZE - DMA1_Channel3->CNDTR) - trigger->sample_index + ADC_BUFF_SIZE) & ADC_BUFF_SIZE_MSK;

        osc_trigger_algo_result result = trigger->type.algo.fn(
            &trigger->type.algo.data,
            adc_buff,
            trigger->sample_index,
            sample_count);

        trigger->sample_index = result.sample_index;

        if (result.eresult == OSC_TRIGGER_ALGO_RESULT_TRIGGER_FOUND) {
            trigger->estate = OSC_TRIGGER_ESTATE_COLLECT_POST_TRIGGER_SAMPLES;
        }
    }

    // Note: Handle the case where DMA interrupt stops sampling due to possible ADC overrun.
    if (oscilloscope->state != OSC_STATE_SAMPLING) {
        osc_transmit_message("Trigger detection algorithm is running too slow in relation to sampling rate.");

        return;
    }

    uint32_t dma1_channel3_cndtr = DMA1_Channel3->CNDTR;

    {
        /* Note:
         * Any DMA transfers that occur during this scope will not
         * be accounted for (CNDTR register changes). In a result,
         * more samples may be collected than necessary. This is not
         * an issue as long as the number of extra samples does not
         * exceed the extra space in the sample buffer.
         */

        uint16_t post_trigger_samples_collected = ((ADC_BUFF_SIZE - dma1_channel3_cndtr) - trigger->sample_index + ADC_BUFF_SIZE) & ADC_BUFF_SIZE_MSK;
        uint16_t samples_to_collect = (oscilloscope->config.sample_count - oscilloscope->config.pre_trigger_sample_count) - post_trigger_samples_collected;

        TIM1->RCR = samples_to_collect - 1; // -1 due to preloading
    }

    /* Note:
     * Wait for UEV to occur after setting the RCR preload register,
     * which will guarantee that RCR shadow register is updated.
     * This part assumes that core and timer use the same clock.
     */
    uint32_t cycles_to_wait = tim_get_cycles_to_uev() << 2; // Note: Divide by 2, since each iteration takes atleast 2 cycles
    for (uint32_t cycle = 0; cycle < cycles_to_wait; ++cycle) {
        __ASM ("NOP");
    }

    TIM1->SR &= ~TIM_SR_UIF;
    __NVIC_ClearPendingIRQ(TIM1_BRK_UP_TRG_COM_IRQn); // Note: Clearing UIF flag from TIM1_SR does not clear the pending bit of TIM1_BRK_UP_TRG_COM_IRQ.

    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

void osc_wait_for_post_trigger_samples(osc* oscilloscope)
{
    if (oscilloscope->state != OSC_STATE_SAMPLING) {
        // Note: Sampling was disabled.
        return;
    }

    while (oscilloscope->trigger.estate <= OSC_TRIGGER_ESTATE_COLLECT_POST_TRIGGER_SAMPLES) {
        __ASM ("NOP");
    }

    // LOG("Captured all post-trigger samples");
}
