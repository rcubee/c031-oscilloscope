#ifndef INC_OSC_H_
#define INC_OSC_H_

#include <stdint.h>
#include <stdbool.h>
#include "osc_protocol.h"
#include "adc.h"
#include "trigger.h"

typedef enum osc_state
{
    OSC_STATE_IDLE,
    OSC_STATE_SAMPLING
} osc_state;

typedef struct osc_config
{
    adc_resolution adc_resolution;
    osc_channel_flags channel1;
    osc_horizontal_scale horizontal_scale;

    uint16_t sample_count; // The total amount of samples to collect
    uint16_t pre_trigger_sample_count; // The amount of pre-trigger samples to collect
} osc_config;

typedef struct osc
{
    osc_state state;
    osc_config config;

    osc_trigger trigger;
} osc;

void osc_init();

void osc_run();

#endif /* INC_OSC_H_ */
