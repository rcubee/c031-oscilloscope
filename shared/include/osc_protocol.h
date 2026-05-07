#ifndef OSC_PROTOCOL_H
#define OSC_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define OSC_HORIZONTAL_SCALE_COUNT (16)

#define OSC_FRAME_SYNC (0x31C0U)

enum {
    OSC_HORIZONTAL_SCALE_10us     = 10,
    OSC_HORIZONTAL_SCALE_20us     = 20,
    OSC_HORIZONTAL_SCALE_50us     = 50,
    OSC_HORIZONTAL_SCALE_100us    = 100,
    OSC_HORIZONTAL_SCALE_200us    = 200,
    OSC_HORIZONTAL_SCALE_500us    = 500,
    OSC_HORIZONTAL_SCALE_1ms      = 1000,
    OSC_HORIZONTAL_SCALE_2ms      = 2000,
    OSC_HORIZONTAL_SCALE_5ms      = 5000,
    OSC_HORIZONTAL_SCALE_10ms     = 10000,
    OSC_HORIZONTAL_SCALE_20ms     = 20000,
    OSC_HORIZONTAL_SCALE_50ms     = 50000,
    OSC_HORIZONTAL_SCALE_100ms    = 100000,
    OSC_HORIZONTAL_SCALE_200ms    = 200000,
    OSC_HORIZONTAL_SCALE_500ms    = 500000,
    OSC_HORIZONTAL_SCALE_1s       = 1000000
};
typedef uint32_t osc_horizontal_scale;

enum
{
    OSC_FRAME_TYPE_INVALID = 0x00,
    OSC_FRAME_TYPE_MESSAGE = 0x01,
    OSC_FRAME_TYPE_START_COLLECTING_SAMPLES = 0x02,
    OSC_FRAME_TYPE_STOP_COLLECTING_SAMPLES = 0x03,
    OSC_FRAME_TYPE_SAMPLES = 0x04,
    OSC_FRAME_TYPE_GET_CONFIG = 0x05,
    OSC_FRAME_TYPE_CONFIG = 0x06
};

typedef uint16_t osc_frame_sync;
typedef uint8_t osc_frame_type;
typedef uint16_t osc_frame_length;

typedef struct __attribute__((packed)) osc_frame_header
{
    osc_frame_sync sync;
    osc_frame_type type;
} osc_frame_header;

typedef struct __attribute__((packed)) osc_frame_message
{
    osc_frame_header header;
    osc_frame_length length;
} osc_frame_message;

typedef struct __attribute__((packed)) osc_frame_samples
{
    osc_frame_header header;
    osc_frame_length length;
} osc_frame_samples;

enum
{
    OSC_CHANNEL_FLAG_ENABLE = (1 << 0)
};
typedef uint8_t osc_channel_flags;

typedef struct __attribute__((packed)) osc_frame_config
{
    osc_frame_header header;
    osc_channel_flags ch1;
    osc_channel_flags ch2;
    osc_horizontal_scale horizontal_scale;
} osc_frame_config;

/**
 * @brief Parse frame header.
 *
 * Parses the frame header to retrieve its type.
 *
 * @param buff Frame buffer.
 * @param buff_len Length of frame buffer.
 * @param type The frame type is returned through this parameter. Set to OSC_FRAME_TYPE_INVALID if no frame header was found.
 * @return If a frame was found, return the index of the frame. Otherwise, return the number of bytes discarded.
 */
size_t osc_frame_parse_header(const uint8_t* buff, size_t buff_capacity, osc_frame_type* type);

osc_frame_message* osc_build_frame_message(uint8_t* buff, uint16_t, const char* message);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OSC_PROTOCOL_H
