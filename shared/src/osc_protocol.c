#include "osc_protocol.h"
#include <stddef.h>
#include <string.h>

uint8_t osc_channel_count(osc_echannel channels)
{
    uint8_t channel_count = 0;

    for (osc_echannel tmp = channels; tmp != 0; tmp >>= 1) {
        channel_count += tmp & 0b1;
    }

    return channel_count;
}

size_t osc_frame_parse_header(const uint8_t* buff, size_t buff_size, osc_frame_type* type)
{
    *type = OSC_FRAME_TYPE_INVALID;

    if (buff == NULL) {
        return 0;
    }

    size_t frame_index = 0;
    while (frame_index < buff_size) {
        if (buff[frame_index] != (OSC_FRAME_SYNC & 0xFF)) {
            ++frame_index;
            continue;
        }

        if (frame_index + 1 == buff_size) {
            return frame_index;
        }

        if (buff[frame_index + 1] != ((OSC_FRAME_SYNC & 0xFF00) >> 8)) {
            frame_index += 2;
            continue;
        }

        if (buff_size - frame_index < sizeof(osc_frame_header)) {
            return frame_index;
        }

        *type = buff[frame_index + offsetof(osc_frame_header, type)];

        return frame_index;
    }

    return buff_size;
}

osc_frame_message* osc_build_frame_message(uint8_t* buff, uint16_t buff_capacity, const char* message)
{
    if (buff == NULL || message == NULL) {
        return NULL;
    }

    if (buff_capacity < sizeof(osc_frame_message)) {
        return NULL;
    }

    uint16_t max_message_length = buff_capacity - sizeof(osc_frame_message);
    uint16_t message_length = strnlen(message, max_message_length + 1);

    // Note: If message_length > max_message_length, no NULL terminator was found.
    if (message_length > max_message_length) {
        return NULL;
    }

    osc_frame_message* frame = (osc_frame_message*)buff;
    frame->header.sync = OSC_FRAME_SYNC;
    frame->header.type = OSC_FRAME_TYPE_MESSAGE;
    frame->length = message_length;

    memcpy((uint8_t*)frame + sizeof(osc_frame_message), message, message_length);

    return frame;
}

osc_frame_samples* osc_build_frame_samples(
    uint8_t* frame_buff,
    uint16_t frame_buff_capacity,
    osc_echannel channels,
    uint16_t* sample_buff,
    uint16_t first_sample_index,
    uint16_t sample_count,
    uint16_t sample_buff_size_msk
)
{
    if (frame_buff == NULL) {
        return NULL;
    }

    if (frame_buff_capacity < sizeof(osc_frame_samples) + sample_count * sizeof(uint16_t)) {
        return NULL;
    }

    uint8_t channel_count = osc_channel_count(channels);

    if (channel_count == 0 || channel_count > 2 || channel_count > sample_count) {
        return NULL;
    }

    uint16_t samples_per_channel = (channel_count == 1) ? sample_count : (sample_count >> 1);

    osc_frame_samples *frame = (osc_frame_samples*)frame_buff;
    frame->header.sync = OSC_FRAME_SYNC;
    frame->header.type = OSC_FRAME_TYPE_SAMPLES;
    frame->channels = channels;
    frame->samples_per_channel = samples_per_channel;

    uint16_t *frame_samples = (uint16_t*)(frame + 1);
    if (channel_count == 1) {
        for (uint16_t i = 0; i < samples_per_channel; ++i) {
            frame_samples[i] = sample_buff[(first_sample_index + i) & sample_buff_size_msk];
        }
    }
    else {
        for (uint16_t i = 0; i < samples_per_channel; ++i) {
            frame_samples[i]                         = sample_buff[(first_sample_index + 2 * i) & sample_buff_size_msk];
            frame_samples[samples_per_channel + i]   = sample_buff[(first_sample_index + 2 * i + 1) & sample_buff_size_msk];
        }
    }

    return frame;
}
