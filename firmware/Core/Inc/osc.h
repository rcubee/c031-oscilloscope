#ifndef INC_OSC_H_
#define INC_OSC_H_

#include <stdint.h>
#include <stdbool.h>
#include "osc_shared.h"
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
    osc_echannel enabled_channels;
    osc_echannel trigger_channel;
    osc_horizontal_scale horizontal_scale;
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
