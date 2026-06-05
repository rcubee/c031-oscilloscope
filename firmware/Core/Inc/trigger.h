#ifndef INC_TRIGGER_H_
#define INC_TRIGGER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum osc_trigger_mode {
    OSC_TRIGGER_MODE_NORM,
    OSC_TRIGGER_MODE_AUTO
} osc_trigger_mode;

typedef enum osc_trigger_etype {
    OSC_TRIGGER_ETYPE_THRESHOLD
} osc_trigger_etype;

typedef enum osc_trigger_algo_eresult {
    OSC_TRIGGER_ALGO_RESULT_TRIGGER_NOT_FOUND = 0U,
    OSC_TRIGGER_ALGO_RESULT_TRIGGER_FOUND
} osc_trigger_algo_eresult;

/* Note:
 * - On trigger not found, sample_index is the index of the sample
 * algorithm checked buffer up to
 * - On trigger found, sample_index is the index of the first post-trigger
 * sample
 */
typedef struct osc_trigger_algo_result {
    osc_trigger_algo_eresult eresult;
    uint16_t scan_pos;
} osc_trigger_algo_result;

typedef osc_trigger_algo_result (*osc_trigger_algo_fn)(
    void* data,
    const uint16_t* sample_buff,
    uint8_t enabled_channel_count,
    uint8_t trigger_channel_seq_pos,
    uint16_t scan_pos,
    uint16_t scan_count
);

typedef struct osc_trigger_algo_threshold {
    /* Config */
    uint16_t threshold_low;
    uint16_t threshold_high;

    /* State */
    bool fell_below_low_threshold;
} osc_trigger_algo_data_threshold;

typedef struct osc_trigger_algo {
    osc_trigger_algo_fn fn;
    union {
        osc_trigger_algo_data_threshold threshold;
    } data;
} osc_trigger_algo;

typedef struct osc_trigger_type {
    osc_trigger_etype etype;
    osc_trigger_algo algo;
} osc_trigger_type;

typedef enum osc_trigger_estate
{
    OSC_TRIGGER_ESTATE_IDLE = 0U,
    OSC_TRIGGER_ESTATE_COLLECT_PRE_TRIGGER_SAMPLES,
    OSC_TRIGGER_ESTATE_WAIT_FOR_TRIGGER,
    OSC_TRIGGER_ESTATE_COLLECT_POST_TRIGGER_SAMPLES,
    OSC_TRIGGER_ESTATE_TRANSMIT_SAMPLES
} osc_trigger_estate;

typedef struct osc_trigger {
    osc_trigger_mode mode;
    osc_trigger_type type;
    osc_trigger_estate estate;

    uint16_t sample_index;

    uint8_t pre_trigger_percentage;
    uint16_t pre_trigger_scan_count;

    uint32_t transfer_complete_count;
} osc_trigger;

/* Algorithm specific functions */

osc_trigger_algo_result osc_trigger_algo_fn_threshold(
    void* data,
    const uint16_t* sample_buff,
    uint8_t enabled_channel_count,
    uint8_t trigger_channel_seq_pos,
    uint16_t scan_pos,
    uint16_t scan_count
);

#endif /* INC_TRIGGER_H_ */
