#include "labeled_dial.hpp"

void LabeledDial::OnValueChanged()
{
    if (_label == nullptr) {
        return;
    }

    _label->setText(QString::fromStdString(IndexToString(value())));
}

LabeledDial::LabeledDial(bool notches_visible)
{
    setNotchesVisible(notches_visible);
}

void LabeledDial::SetLabel(QLabel* label)
{
    if (label == nullptr) {
        throw std::runtime_error("");
    }

    _label = label;

    connect(this, &QDial::valueChanged, this, &LabeledDial::OnValueChanged);

    // Note: Update the label for the first time
    OnValueChanged();
}
