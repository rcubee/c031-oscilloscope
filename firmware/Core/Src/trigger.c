#include "trigger.h"
#include "adc.h"

osc_trigger_algo_result osc_trigger_algo_fn_threshold(
    void* data,
    const uint16_t* sample_buff,
    uint8_t enabled_channel_count,
    uint8_t trigger_channel_seq_pos,
    uint16_t scan_pos,
    uint16_t scan_count
) {
    osc_trigger_algo_data_threshold* algo =
        (osc_trigger_algo_data_threshold*)data;
    uint16_t scans_left = scan_count;

    while (scans_left--) {
        uint16_t sample_index = (scan_pos + trigger_channel_seq_pos) & ADC_BUFF_SIZE_MSK;
        uint16_t sample = sample_buff[sample_index];

        if (!algo->fell_below_low_threshold) {
            if (sample <= algo->threshold_low) {
                algo->fell_below_low_threshold = true;
            }
        }
        else if (sample >= algo->threshold_high) {
            return (osc_trigger_algo_result) {
                .eresult = OSC_TRIGGER_ALGO_RESULT_TRIGGER_FOUND,
                .scan_pos = scan_pos
            };
        }

        scan_pos = (scan_pos + enabled_channel_count) & ADC_BUFF_SIZE_MSK;
    }

    return (osc_trigger_algo_result) {
        .eresult = OSC_TRIGGER_ALGO_RESULT_TRIGGER_NOT_FOUND,
        .scan_pos = scan_pos
    };
}
