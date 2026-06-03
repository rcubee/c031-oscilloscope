#ifndef OSC_SHARED_H
#define OSC_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define OSC_HORIZONTAL_SCALE_COUNT (16)

#define OSC_FRAME_SYNC (0x31C0U)
typedef uint16_t osc_frame_sync;

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
typedef uint8_t osc_frame_type;

typedef uint16_t osc_frame_length;

typedef struct __attribute__((packed)) osc_frame_header
{
    osc_frame_sync sync;
    osc_frame_type type;
} osc_frame_header;

enum
{
    OSC_ECHANNEL_1 = (1U << 0),
    OSC_ECHANNEL_2 = (1U << 1)
};
typedef uint8_t osc_echannel;

/* Note:
 * Using packed attribute for frame structs;
 * 1. It ensures there are no alignment differences between firmware and GUI.
 * 2. Compiler doesn't assume fields are aligned inside the struct - it removes the need for aligning frame buffers.
 */

typedef struct __attribute__((packed)) osc_frame_message
{
    osc_frame_header header;
    osc_frame_length length;
} osc_frame_message;

typedef struct __attribute__((packed)) osc_frame_config
{
    osc_frame_header header;
    osc_echannel channels_enabled;
    osc_horizontal_scale horizontal_scale;
} osc_frame_config;

// Note: For samples to be safely appended at the end of the frame, it should be uint16_t aligned.
typedef struct __attribute__((packed, aligned(sizeof(uint16_t)))) osc_frame_samples
{
    osc_frame_header header;
    osc_echannel channels;
    osc_frame_length samples_per_channel;
} osc_frame_samples;

uint8_t osc_channel_count(osc_echannel channels);

/**
 * @brief Parse frame header.
 *
 * Parses the frame header to retrieve its type.
 *
 * @param buff Frame buffer.
 * @param buff_size Size of the frame buffer.
 * @param type The frame type is returned through this parameter. Set to OSC_FRAME_TYPE_INVALID if no frame header was found.
 * @return If a frame was found, return the index of the frame. Otherwise, return the number of bytes discarded.
 */
size_t osc_frame_parse_header(const uint8_t* buff, size_t buff_size, osc_frame_type* type);

/**
 * @brief Build message frame.
 *
 * @param buff Frame buffer.
 * @param buff_capacity Capacity of the frame buffer.
 * @param message NULL-terminated string.
 * @return If a frame was found, return the index of the frame. Otherwise, return the number of bytes discarded.
 */
osc_frame_message* osc_build_frame_message(uint8_t* buff, uint16_t buff_capacity, const char* message);

/**
 * @brief Build samples frame.
 *
 * Build easy to parse frame from samples collected by one or more channels.
 *
 * @warning frame_buff has to be aligned to osc_frame_samples.
 *
 * @param frame_buff Frame buffer.
 * @param frame_buff_capacity Capacity of the frame buffer.
 * @param sample_buff_size_msk Mask of sample circular buffer capacity (must be a power of 2).
 * @return If a frame was found, return the index of the frame. Otherwise, return the number of bytes discarded.
 */
osc_frame_samples* osc_build_frame_samples(
    uint8_t* frame_buff,
    uint16_t frame_buff_capacity,
    osc_echannel channels,
    uint16_t* sample_buff,
    uint16_t first_sample_index,
    uint16_t sample_count,
    uint16_t sample_buff_size_msk
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OSC_SHARED_H
