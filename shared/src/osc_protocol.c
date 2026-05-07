#include "osc_protocol.h"
#include <stddef.h>
#include <string.h>

size_t osc_frame_parse_header(const uint8_t* buff, size_t buff_capacity, osc_frame_type* type)
{
    *type = OSC_FRAME_TYPE_INVALID;

    if (buff == NULL) {
        return 0;
    }

    size_t frame_index = 0;
    while (frame_index < buff_capacity) {
        if (buff[frame_index] != (OSC_FRAME_SYNC & 0xFF)) {
            ++frame_index;
            continue;
        }

        if (frame_index + 1 == buff_capacity) {
            return frame_index;
        }

        if (buff[frame_index + 1] != ((OSC_FRAME_SYNC & 0xFF00) >> 8)) {
            frame_index += 2;
            continue;
        }

        if (buff_capacity - frame_index < sizeof(osc_frame_header)) {
            return frame_index;
        }

        *type = buff[frame_index + offsetof(osc_frame_header, type)];

        return frame_index;
    }

    return buff_capacity;
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
