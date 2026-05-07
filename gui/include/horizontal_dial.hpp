#pragma once

#include "labeled_dial.hpp"
#include "osc_protocol.h"

class HorizontalDial : public LabeledDial
{
    Q_OBJECT
private:
    std::string IndexToString(int index) override;
    osc_horizontal_scale IndexToHorizontalScale(int index);
    int HorizontalScaleToIndex(osc_horizontal_scale horizontal_scale);

public:
    explicit HorizontalDial(QWidget *parent = nullptr);

    osc_horizontal_scale GetHorizontalScale();
    void SetHorizontalScale(osc_horizontal_scale horizontal_scale);
};
