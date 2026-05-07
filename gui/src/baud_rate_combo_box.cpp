#include "baud_rate_combo_box.hpp"
#include "oscilloscope.hpp"

// void BaudRateComboBox::OnCurrentIndexChanged(int index)
// {
//     QSerialPort::BaudRate baud_rate = GetBaudRate();
//
//     emit BaudRateChanged(baud_rate);
// }

BaudRateComboBox::BaudRateComboBox(QWidget* parent)
{
    for (const auto baud_rate : { QSerialPort::BaudRate(BAUD_RATE_MIN), QSerialPort::BaudRate(BAUD_RATE_DEFAULT), QSerialPort::BaudRate(BAUD_RATE_MAX) }) {
        QString text = QString::number(static_cast<int>(baud_rate));
        addItem(text, baud_rate);
    }

    // Set BAUD_RATE_DEFAULT as currently selected option
    setCurrentIndex(1);

    // connect(this, &QComboBox::currentIndexChanged, this, &BaudRateComboBox::OnCurrentIndexChanged);
}

QSerialPort::BaudRate BaudRateComboBox::GetBaudRate()
{
    return itemData(currentIndex()).value<QSerialPort::BaudRate>();
}
