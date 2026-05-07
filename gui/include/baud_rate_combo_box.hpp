#pragma once

#include <QComboBox>
#include <QSerialPort>

class BaudRateComboBox : public QComboBox
{
    Q_OBJECT;
// private slots:
//     void OnCurrentIndexChanged(int index);
//
// signals:
//     void BaudRateChanged(QSerialPort::BaudRate baud_rate);

public:
    explicit BaudRateComboBox(QWidget* parent = nullptr);

    QSerialPort::BaudRate GetBaudRate();
};
