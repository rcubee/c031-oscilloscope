#include "adc.h"
#include <assert.h>
#include "debug.h"
#include "dma.h"
#include "gpio.h"
#include "osc.h"
#include "protocol.h"
#include "pwm.h"
#include "tim.h"
#include "stm32c031xx.h"
#include "systick.h"
#include "usart.h"

#define DIV_COUNT 10
#define MAX_SAMPLE_COUNT 1000

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
        .channels_enabled = 0,
        .horizontal_scale = OSC_HORIZONTAL_SCALE_1ms
    };

    return config;
}

static osc_trigger osc_get_default_trigger() {
    osc_trigger trigger = {
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

        .pre_trigger_percentage = 25,

        .transfer_complete_count = 0
    };

    return trigger;
}

/*
 * @brief Returns the number of samples collected after trigger->sample_index.
 */
static uint16_t osc_get_post_trigger_sample_count(osc* oscilloscope)
{
    return ((ADC_BUFF_SIZE - DMA1_Channel3->CNDTR) - oscilloscope->trigger.sample_index + ADC_BUFF_SIZE) & ADC_BUFF_SIZE_MSK;
}

/*
 * @brief Returns the number of scans done after trigger->sample_index.
 */
static uint16_t osc_get_post_trigger_scan_count(osc* oscilloscope)
{
    return osc_get_post_trigger_sample_count(oscilloscope) / osc_channel_count(oscilloscope->config.channels_enabled);
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

    bool processing_first_half = oscilloscope->trigger.sample_index < (ADC_BUFF_SIZE >> 1);
    bool transferring_to_first_half = DMA1->ISR & DMA_ISR_TCIF3;

    if ((transferring_to_first_half && processing_first_half) || (!transferring_to_first_half && !processing_first_half)) {
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
    trigger->pre_trigger_scan_count = 0;
    trigger->transfer_complete_count = 0;

    // TODO: Move
    trigger->type.algo.data.threshold.fell_below_low_threshold = false;
}

void osc_init()
{
#ifdef DEBUG
    debug_init();
#endif // DEBUG

    LOG("Build date: %s", __TIMESTAMP__);

    osc* oscilloscope = osc_get();
    *oscilloscope = (osc) {
        .state = OSC_STATE_IDLE,
        .config = osc_get_default_config(),
        .trigger = osc_get_default_trigger()
    };

    adc_init();
    dma_init();
    gpio_init();
    pwm_init();
    systick_init();
    tim_init();
    usart_init();
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

void osc_wait_for_pre_trigger_scans(osc* oscilloscope, uint16_t total_scan_count);
void osc_wait_for_trigger(osc* oscilloscope, uint8_t timeout_ms, uint16_t samples_per_channel);
void osc_wait_for_post_trigger_samples(osc* oscilloscope);

void osc_sample()
{
    osc* oscilloscope = osc_get();

    if (oscilloscope->state != OSC_STATE_SAMPLING || oscilloscope->config.channels_enabled == 0) {
        return;
    }

    osc_trigger_reset(&oscilloscope->trigger);

    adc_acq_params acq_params = adc_calculate_acq_params(
        DIV_COUNT * oscilloscope->config.horizontal_scale,
        MAX_SAMPLE_COUNT,
        osc_channel_count(oscilloscope->config.channels_enabled)
    );

    tim_configure_for_sampling(acq_params.ext_trig_interval); // Note: ADC & Timer use the same clock.
    adc_configure_for_sampling(oscilloscope->config.channels_enabled, ADC_SAMPLING_TIME_3_5, oscilloscope->config.adc_resolution);
    dma_configure_for_sampling();

    /* Note: When sampling starts, software must process collected samples
     * (update the sample_index) fast enough, so that ADC overrun doesn't occur.
     */
    tim_enable();

    osc_wait_for_pre_trigger_scans(oscilloscope, acq_params.scan_count);
    osc_wait_for_trigger(oscilloscope, 100, acq_params.scan_count);
    osc_wait_for_post_trigger_samples(oscilloscope);

    osc_transmit_samples(oscilloscope, osc_channel_count(oscilloscope->config.channels_enabled) * acq_params.scan_count);
}

void osc_wait_for_pre_trigger_scans(osc* oscilloscope, uint16_t total_scan_count)
{
    osc_trigger* trigger = &oscilloscope->trigger;

    trigger->estate = OSC_TRIGGER_ESTATE_COLLECT_PRE_TRIGGER_SAMPLES;

    uint16_t scans_done = 0;
    trigger->pre_trigger_scan_count = (total_scan_count * trigger->pre_trigger_percentage) / 100;

    while (scans_done < trigger->pre_trigger_scan_count) {
        uint16_t post_trigger_scans = osc_get_post_trigger_scan_count(oscilloscope);
        uint8_t enabled_channel_count = osc_channel_count(oscilloscope->config.channels_enabled);

        trigger->sample_index += post_trigger_scans * enabled_channel_count;

        scans_done += post_trigger_scans;
    }
}

void osc_wait_for_trigger(osc* oscilloscope, uint8_t timeout_ms, uint16_t samples_per_channel)
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

        uint16_t sample_count = osc_get_post_trigger_sample_count(oscilloscope);

        // TODO: Capture trigger on specific channel
        osc_trigger_algo_result result = trigger->type.algo.fn(
            &trigger->type.algo.data,
            adc_buff,
            osc_channel_count(oscilloscope->config.channels_enabled),
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

    /* Note:
     * Any DMA transfers that occur after calculating scans_done
     * will not be accounted for (ADC scans are ongoing). As a result,
     * more samples may be collected than necessary. This is not
     * an issue as long as the number of extra samples does not
     * exceed the extra space in the sample buffer.
     */

    uint16_t scans_done = osc_get_post_trigger_scan_count(oscilloscope);
    uint16_t scans_to_do = samples_per_channel - scans_done - oscilloscope->trigger.pre_trigger_scan_count;

    TIM1->RCR = scans_to_do - 1; // -1 due to preloading

    /* Note:
     * Wait for UEV to occur after setting the RCR preload register,
     * which will guarantee that RCR shadow register is updated.
     * This part assumes that core and timer use the same clock.
     */
    uint32_t cycles_to_wait = tim_get_cycles_to_uev() >> 1; // Note: Divide by 2, since each iteration takes atleast 2 cycles
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
