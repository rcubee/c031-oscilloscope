#pragma once

#include <QtWidgets/QDial>
#include <QtWidgets/QLabel>

class LabeledDial : public QDial
{
    Q_OBJECT
private slots:
    void OnValueChanged();

private:
    QLabel* _label;

    virtual std::string IndexToString(int index) = 0;

public:
    // TODO: Set range & default value
    LabeledDial(bool notches_visible);

    void SetLabel(QLabel* label);
};
