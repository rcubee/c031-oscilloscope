#include "serial_connection.hpp"
#include "osc_protocol.h"
#include <QDebug>

bool SerialConnection::WriteFrame(void* frame, size_t frame_size)
{
    if (!isOpen()) {
        return false;
    }

    bool result = writeData(reinterpret_cast<const char*>(frame), frame_size) == frame_size;
    if (!result) {
        qDebug() << "Failed writing to " << this->portName() << ": "<< errorString();
    }

    return result;
}


SerialConnection::SerialConnection()
    : read_buffer()
{
    read_buffer.reserve(READ_BUFFER_CAPACITY);

    connect(this, &QSerialPort::readyRead, this, &SerialConnection::OnReadyRead);
}

SerialConnection::~SerialConnection() { }

void SerialConnection::Configure(const QSerialPort::DataBits data_bits, const QSerialPort::Parity parity, const QSerialPort::StopBits stop_bits, const QSerialPort::BaudRate baud_rate)
{
    setDataBits(data_bits);
    setParity(parity);
    setStopBits(stop_bits);

    setBaudRate(baud_rate);
}

bool SerialConnection::IsOpen()
{
    return isOpen();
}

bool SerialConnection::Open(const QString& device_path)
{
    Close();

    setPortName(device_path);

    return open(QIODevice::OpenModeFlag::ReadWrite);
}

void SerialConnection::Close()
{
    if (isOpen()) {
        close();
    }
}

void SerialConnection::Flush()
{
    clear(Direction::Input);
    read_buffer.clear();

    clear(Direction::Output);
}

void SerialConnection::OnReadyRead()
{
    Read();

    osc_frame_type type;
    size_t frame_index = osc_frame_parse_header(read_buffer.data(), read_buffer.size(), &type);

    read_buffer.erase(read_buffer.begin(), read_buffer.begin() + frame_index);

    if (type == OSC_FRAME_TYPE_MESSAGE) {
        osc_frame_message* frame_message = (osc_frame_message*)read_buffer.data();

        std::string message((char*)frame_message + sizeof(osc_frame_message), frame_message->length);

        emit SignalFrameMessage(message);

        read_buffer.erase(read_buffer.begin(), read_buffer.begin() + sizeof(osc_frame_message));
    }
    else if (type == OSC_FRAME_TYPE_SAMPLES) {
        if (read_buffer.size() < sizeof(osc_frame_samples)) {
            // Note: Wait for the rest of the packet header
            return;
        }

        osc_frame_samples* frame = reinterpret_cast<osc_frame_samples*>(read_buffer.data());

        uint8_t channel_count = osc_channel_count(frame->channels);

        size_t frame_size = sizeof(osc_frame_samples) + channel_count * frame->samples_per_channel * sizeof(uint16_t);
        size_t sample_count = channel_count * frame->samples_per_channel;
        if (read_buffer.size() < frame_size) {
            // Note: Wait for the packet payload
            return;
        }

        std::vector<ChannelSamples> channel_samples_vec;

        if (frame->channels & OSC_ECHANNEL_1) {
            channel_samples_vec.emplace_back(EChannel::_1, (std::vector<std::uint16_t>){ });
        }

        if (frame->channels & OSC_ECHANNEL_2) {
            channel_samples_vec.emplace_back(EChannel::_2, (std::vector<std::uint16_t>){ });
        }

        // Copy for each channel samples
        for (size_t channel = 0; channel < channel_samples_vec.size(); ++channel) {
            auto& channel_samples = channel_samples_vec.at(channel);

            channel_samples.samples.resize(frame->samples_per_channel);

            for (size_t i = 0; i < frame->samples_per_channel; ++i) {
                const std::uint8_t* payload = (uint8_t*)frame + sizeof(osc_frame_samples) + frame->samples_per_channel * sizeof(uint16_t) * channel;

                const std::uint16_t sample = payload[2 * i] | (payload[2 * i + 1] << 8);

                channel_samples.samples.at(i) = sample;
            }
        }

        emit SignalFrameSamples(channel_samples_vec);

        read_buffer.erase(read_buffer.begin(), read_buffer.begin() + frame_size);
    }
    else if (type == OSC_FRAME_TYPE_CONFIG) {
        if (read_buffer.size() < sizeof(osc_frame_config)) {
            return;
        }

        osc_frame_config frame_config;
        memcpy(&frame_config, (osc_frame_config*)read_buffer.data(), sizeof(osc_frame_config));

        emit SignalFrameConfig(frame_config);

        read_buffer.erase(read_buffer.begin(), read_buffer.begin() + sizeof(osc_frame_config));
    }
}

void SerialConnection::Read()
{
    size_t bytes_available = bytesAvailable();
    if (bytes_available < 1) {
        return;
    }

    if (read_buffer.size() + bytes_available > READ_BUFFER_CAPACITY) {
        Flush();
        return;
    }

    auto byte_array = readAll();

    // qDebug() << "Read" << byte_array.size() << " bytes";

    read_buffer.insert(read_buffer.end(), byte_array.begin(), byte_array.end());
}

void SerialConnection::FrameStartCollectingSamples()
{
    osc_frame_header frame_header;
    frame_header.sync = OSC_FRAME_SYNC;
    frame_header.type = OSC_FRAME_TYPE_START_COLLECTING_SAMPLES;

    WriteFrame(&frame_header, sizeof(frame_header));
}

void SerialConnection::FrameStopCollectingSamples()
{
    osc_frame_header frame_header;
    frame_header.sync = OSC_FRAME_SYNC;
    frame_header.type = OSC_FRAME_TYPE_STOP_COLLECTING_SAMPLES;

    WriteFrame(&frame_header, sizeof(frame_header));
}

void SerialConnection::FrameGetConfig()
{
    osc_frame_header frame_header;
    frame_header.sync = OSC_FRAME_SYNC;
    frame_header.type = OSC_FRAME_TYPE_GET_CONFIG;

    WriteFrame(&frame_header, sizeof(frame_header));
}

void SerialConnection::FrameConfig(osc_echannel channels_enabled, osc_horizontal_scale horizontal_scale)
{
    osc_frame_config frame_config;
    frame_config.header.sync = OSC_FRAME_SYNC;
    frame_config.header.type = OSC_FRAME_TYPE_CONFIG;
    frame_config.channels_enabled = channels_enabled;
    frame_config.horizontal_scale = horizontal_scale;

    WriteFrame(&frame_config, sizeof(frame_config));
}
