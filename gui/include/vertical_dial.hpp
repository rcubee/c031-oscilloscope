#pragma once

#include "labeled_dial.hpp"

enum class VerticalScale
{
    _5 = 0,
    _2,
    _1,
    _500m,
    _200m,
    _100m,
    _50m,
    _20m,
    _10m,
    _5m,
    _2m,
    _1m,
    _500u,

    Count
};

float VerticalScaleToMultiplier(VerticalScale vertical_scale);

class VerticalDial : public LabeledDial
{
    Q_OBJECT
private:
    std::string IndexToString(int index) override;
    VerticalScale IndexToVerticalScale(int index);

public:
    explicit VerticalDial(QWidget *parent = nullptr);

    VerticalScale GetVerticalScale();
};
