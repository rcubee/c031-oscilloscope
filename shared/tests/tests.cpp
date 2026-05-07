#include <gtest/gtest.h>
#include "osc_protocol.h"
#include <vector>

TEST(ParseFrameHeader, Empty) {
    std::vector<std::uint8_t> buff;

    // Note: std::vector<...>.data() retuns nullptr when not initialized
    buff.resize(1);
    buff.resize(0);

    osc_frame_type type;
    size_t frame_index = osc_frame_parse_header(buff.data(), buff.size(), &type);

    EXPECT_EQ(type, OSC_FRAME_TYPE_INVALID);
    EXPECT_EQ(frame_index, 0);
}

TEST(ParseFrameHeader, NoFrame) {
    std::vector<std::uint8_t> buff {
        0x00,
        0x11,
        (OSC_FRAME_SYNC & 0xFF),
        0x33,
        0x44
    };

    osc_frame_type type;
    size_t frame_index = osc_frame_parse_header(buff.data(), buff.size(), &type);

    EXPECT_EQ(type, OSC_FRAME_TYPE_INVALID);
    EXPECT_EQ(frame_index, buff.size());
}

TEST(ParseFrameHeader, InvalidType) {
    std::vector<std::uint8_t> buff {
        (OSC_FRAME_SYNC & 0xFF),
        ((OSC_FRAME_SYNC & 0xFF00) >> 8),
        OSC_FRAME_TYPE_INVALID
    };

    osc_frame_type type;
    size_t frame_index = osc_frame_parse_header(buff.data(), buff.size(), &type);

    EXPECT_EQ(type, OSC_FRAME_TYPE_INVALID);
    EXPECT_EQ(frame_index, 0);
}

TEST(ParseFrameHeader, StartCollectingSamplesType) {
    std::vector<std::uint8_t> buff {
        0x00,
        0x11,
        (OSC_FRAME_SYNC & 0xFF),
        ((OSC_FRAME_SYNC & 0xFF00) >> 8),
        OSC_FRAME_TYPE_START_COLLECTING_SAMPLES,
        0x55,
        0x66,
    };

    osc_frame_type type;
    size_t frame_index = osc_frame_parse_header(buff.data(), buff.size(), &type);

    EXPECT_EQ(type, OSC_FRAME_TYPE_START_COLLECTING_SAMPLES);
    EXPECT_EQ(frame_index, 2);
}
