#include "trigger.h"

osc_trigger_algo_result osc_trigger_algo_fn_threshold(
    void* data,
    const uint16_t* sample_buff,
    uint8_t channel_count,
    uint16_t sample_index,
    uint16_t sample_count)
{
    osc_trigger_algo_data_threshold* algo =
        (osc_trigger_algo_data_threshold*)data;
    uint16_t samples_left = sample_count;

    while (samples_left--) {
        if (channel_count > 1 && sample_index & 0b1) {
            sample_index = (sample_index + 1) & ADC_BUFF_SIZE_MSK;
            continue;
        }

        uint16_t sample = sample_buff[sample_index];

        if (!algo->fell_below_low_threshold) {
            if (sample <= algo->threshold_low) {
                algo->fell_below_low_threshold = true;
            }
        }
        else if (sample >= algo->threshold_high) {
            return (osc_trigger_algo_result) {
                .eresult = OSC_TRIGGER_ALGO_RESULT_TRIGGER_FOUND,
                .sample_index = sample_index
            };
        }

        sample_index = (sample_index + 1) & ADC_BUFF_SIZE_MSK;
    }

    return (osc_trigger_algo_result) {
        .eresult = OSC_TRIGGER_ALGO_RESULT_TRIGGER_NOT_FOUND,
        .sample_index = sample_index
    };
}
