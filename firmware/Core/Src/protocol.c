#include "protocol.h"
#include "pwm.h"
#include <stdint.h>
#include "usart.h"

#define FRAME_BUFF_CAPACITY (sizeof(osc_frame_samples) + ADC_BUFF_SIZE * sizeof(uint16_t))

void osc_handle_protocol(osc* oscilloscope)
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
        frame.channels_enabled = oscilloscope->config.channels_enabled;
        frame.horizontal_scale = oscilloscope->config.horizontal_scale;

        usart_transmit_dma((uint8_t*)&frame, sizeof(osc_frame_config));

        break;
    }

    case OSC_FRAME_TYPE_CONFIG:
    {
        osc_frame_config* frame = (osc_frame_config*)(buff + index);

        oscilloscope->config.channels_enabled = frame->channels_enabled;
        oscilloscope->config.horizontal_scale = frame->horizontal_scale;

        break;
    }

    default: break;
    }
}

void osc_transmit_message(const char* message)
{
    uint8_t buff[128];

    osc_frame_message *frame = osc_build_frame_message(buff, sizeof(buff), message);

    usart_transmit_dma((uint8_t*)frame, sizeof(osc_frame_message) + frame->length);
}

void osc_transmit_samples(osc* oscilloscope, uint16_t samples_collected)
{
    if (oscilloscope->state != OSC_STATE_SAMPLING) {
        // Note: Sampling was disabled.
        return;
    }

    static uint8_t frame_buff[FRAME_BUFF_CAPACITY];

    uint16_t frame_size = sizeof(osc_frame_samples) + samples_collected * sizeof(uint16_t);
    osc_frame_samples *frame = osc_build_frame_samples(
        frame_buff,
        FRAME_BUFF_CAPACITY,
        oscilloscope->config.channels_enabled,
        adc_get_buff(),
        oscilloscope->trigger.sample_index - (oscilloscope->trigger.pre_trigger_scan_count * osc_channel_count(oscilloscope->config.channels_enabled)),
        samples_collected,
        ADC_BUFF_SIZE_MSK
    );

    assert(frame != NULL);

    usart_transmit_dma((uint8_t*)frame, frame_size);

    oscilloscope->trigger.estate = OSC_TRIGGER_ESTATE_IDLE;

    // LOG("Transmitted all samples");
}
