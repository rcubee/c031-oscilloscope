#pragma once

#include <QColor>
#include <QLineSeries>
#include <QSerialPort>
#include <QValueAxis>
#include <vertical_dial.hpp>
#include "osc_shared.h"

static constexpr float VOLTAGE_MIN = 0.0f;
static constexpr float VOLTAGE_MAX = 3.3f;
static constexpr float VPP = VOLTAGE_MAX - VOLTAGE_MIN;

static constexpr qint32 BAUD_RATE_MIN(115200);
static constexpr qint32 BAUD_RATE_DEFAULT(1000000);
static constexpr qint32 BAUD_RATE_MAX(2000000);

static constexpr QColor COLOR_CHANNEL_1(255, 255, 0);
static constexpr QColor COLOR_CHANNEL_2(0, 0, 255);
static constexpr QColor COLOR_CHANNEL_DISABLED(127, 127, 127);

enum class EChannel
{
    Invalid = 0,
    _1,
    _2
};

class Channel
{
private:
    EChannel _echannel;
    bool _enabled;
    QLineSeries* _series;
    VerticalScale _vertical_scale;

public:
    Channel(EChannel echannel);
    ~Channel();

    bool IsEnabled();
    void SetEnabled();
    void SetDisabled();

    void AttachAxis(QValueAxis* x_axis, QValueAxis* y_axis);
    QLineSeries* GetSeries();

    void Replace(const std::vector<uint16_t>& samples, qreal vertical_position);

    void SetVerticalScale(VerticalScale vertical_scale);
};

constexpr QColor GetChannelColor(EChannel channel)
{
    switch (channel) {
        case EChannel::_1: return COLOR_CHANNEL_1;
        case EChannel::_2: return COLOR_CHANNEL_2;
        default: throw std::runtime_error("Invalid channel");
    }
};

class ChannelSamples
{
public:
    EChannel channel;
    std::vector<std::uint16_t> samples;

    ChannelSamples(EChannel channel, std::vector<std::uint16_t> samples);
};
