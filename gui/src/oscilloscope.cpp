#include "oscilloscope.hpp"

Channel::Channel(EChannel echannel)
    :
    _echannel(echannel),
    _enabled(),
    _series(new QLineSeries),
    _vertical_scale(VerticalScale::_1)
{
    _series->setPen(QPen(GetChannelColor(echannel)));

    _series->setName(QString::fromStdString("Channel " + std::to_string(static_cast<int>(echannel))));
}

Channel::~Channel()
{
    if (_series) {
        delete _series;
    }
}

bool Channel::IsEnabled()
{
    return _enabled;
}

void Channel::SetEnabled()
{
    _enabled = true;
}

void Channel::SetDisabled()
{
    _enabled = false;

    _series->clear();
}

void Channel::AttachAxis(QValueAxis* x_axis, QValueAxis* y_axis)
{
    _series->attachAxis(x_axis);
    _series->attachAxis(y_axis);
}

QLineSeries* Channel::GetSeries()
{
    return _series;
}

void Channel::Replace(const std::vector<uint16_t>& samples, qreal vertical_position)
{
    if (!_enabled) {
        return;
    }

    QVector<QPointF> points;
    points.resize(samples.size());

    for (size_t i = 0; i < samples.size(); ++i) {
        // const qreal x = i;

        QValueAxis *x_axis = qobject_cast<QValueAxis*>(_series->attachedAxes().first());
        const qreal x = static_cast<qreal>(i) * (x_axis->max() / (samples.size() - 1));

        qreal y = ((float)samples.at(i) / 1023.0f) * VPP * (VerticalScaleToMultiplier(_vertical_scale));
        y += vertical_position;
        const QPointF point(x, y);

        points[i] = point;
    }

    _series->replace(points);
}

void Channel::SetVerticalScale(VerticalScale vertical_scale)
{
    _vertical_scale = vertical_scale;
}

ChannelSamples::ChannelSamples(EChannel channel, std::vector<std::uint16_t> samples)
    : channel(channel), samples(samples) {  }
