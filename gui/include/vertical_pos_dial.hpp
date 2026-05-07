#pragma once

#include "labeled_dial.hpp"

class VerticalPosDial : public LabeledDial
{
    Q_OBJECT;
private:
    std::string IndexToString(int index) override;

protected:
    void mousePressEvent(QMouseEvent* event) override;

public:
    explicit VerticalPosDial(QWidget* parent = nullptr);

    float GetPosition();
};
