#include "channel_button.hpp"

void ChannelButton::OnPushButtonClicked()
{
    if (_channel == EChannel::Invalid) {
        throw std::runtime_error("Invalid channel");
    }

    _enabled = !_enabled;

    SetStyleSheet();

    emit Toggled(this, _channel, _enabled);
}

void ChannelButton::SetStyleSheet()
{
    QColor color = _enabled ? _color : COLOR_CHANNEL_DISABLED;

    setStyleSheet("QPushButton { background-color: " + color.name(QColor::NameFormat::HexRgb) + "; color: black; font-weight: bold; }");
}

ChannelButton::ChannelButton(QWidget* parent)
    :
    _channel(EChannel::Invalid),
    _enabled(false)
{

}

bool ChannelButton::IsEnabled()
{
    return _enabled;
}

void ChannelButton::SetChannel(EChannel channel)
{
    _channel = channel;

    _color = GetChannelColor(channel);
    SetStyleSheet();

    connect(this, &QPushButton::clicked, this, &ChannelButton::OnPushButtonClicked);
}

void ChannelButton::SetEnabled()
{
    _enabled = true;

    SetStyleSheet();
}

void ChannelButton::SetDisabled()
{
    _enabled = false;

    SetStyleSheet();
}
