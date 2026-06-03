#pragma once

#include "osc_shared.h"
#include "oscilloscope.hpp"
#include <QMutex>
#include <QMutexLocker>
#include <QSerialPort>

class SerialConnection : public QSerialPort
{
    Q_OBJECT;
private:
    static constexpr size_t READ_BUFFER_CAPACITY = (10 * 1024); // 10 kB

    std::vector<std::uint8_t> read_buffer;

    void Read();
    void OnReadyRead();

    bool WriteFrame(void* frame, size_t frame_size);

signals:
    void SignalFrameMessage(const std::string& message);
    void SignalFrameSamples(const std::vector<ChannelSamples>& channel_samples);
    void SignalFrameConfig(osc_frame_config frame_config);

public:
    SerialConnection();
    ~SerialConnection();

    void Configure(const QSerialPort::DataBits data_bits, const QSerialPort::Parity parity, const QSerialPort::StopBits stop_bits, const QSerialPort::BaudRate baud_rate);

    bool IsOpen();
    [[nodiscard]] bool Open(const QString& device_path);
    void Close();
    void Flush();

    void FrameStartCollectingSamples();
    void FrameStopCollectingSamples();
    void FrameGetConfig();
    void FrameConfig(osc_echannel channels_enabled, osc_horizontal_scale horizontal_scale);
};
