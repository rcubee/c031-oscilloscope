#include "vertical_pos_dial.hpp"
#include <sstream>
#include <iomanip>
#include <QMouseEvent>

std::string VerticalPosDial::IndexToString(int index)
{
    float position = GetPosition();

    std::stringstream ss;

    ss << std::fixed << std::setprecision(1) << position;

    return ss.str() + " graticules";
}

void VerticalPosDial::mousePressEvent(QMouseEvent* event)
{
    // Note: Reset to default value on right mouse button

    if (event->button() == Qt::MouseButton::RightButton) {
        setValue(maximum() / 2);
    } else {
        QDial::mousePressEvent(event);
    }
}

VerticalPosDial::VerticalPosDial(QWidget* parent)
    : LabeledDial(false)
{
    setRange(0, 20 * 10); // Offset from -10 to 10 graticules (overall 20), 10 points each
    setValue(maximum() / 2);
}

float VerticalPosDial::GetPosition()
{
    float vertical_position = (value() - 100) / 10.f;

    return vertical_position;
}
