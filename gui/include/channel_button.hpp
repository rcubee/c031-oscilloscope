#pragma once

#include "oscilloscope.hpp"
#include <QColor>
#include <QPushButton>

class ChannelButton : public QPushButton
{
    Q_OBJECT;
private slots:
    void OnPushButtonClicked();

private:
    EChannel _channel;
    bool _enabled;

    QColor _color;

    void SetStyleSheet();

signals:
    void Toggled(ChannelButton* channel_button, EChannel channel, bool enabled);

public:
    explicit ChannelButton(QWidget* parent = nullptr);

    void SetChannel(EChannel channel);

    bool IsEnabled();
    void SetEnabled();
    void SetDisabled();
};
