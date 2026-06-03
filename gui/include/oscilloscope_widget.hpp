#include "serial_connection.hpp"
#include "ui_oscilloscope.h"
#include <QLineSeries>
#include <QValueAxis>

class OscilloscopeWidget : public QWidget
{
    Q_OBJECT
private slots:
    void OnSerialConnectionOpened();
    void OnFrameMessage(const std::string& message);
    void OnSerialConnectionGetConfig(osc_frame_config config);
    void OnSerialConnectionErrorOccured(QSerialPort::SerialPortError serial_port_error);
    void OnSerialConnectionClosed();

    void OnConnectButtonReleased();
    void OnChannelButtonToggled(ChannelButton* channel_button, EChannel channel, bool enabled);
    void OnVerticalDialChanged(int);
    void OnHorizontalScaleChanged(int);

private:
    Ui::oscilloscope ui;
    QValueAxis* x_axis;
    QValueAxis* y_axis;
    QChart* chart;

    std::array<Channel, 2> channels;
    Channel& channel1;
    Channel& channel2;

    SerialConnection serial_connection;

    void UpdateConfig();

    void ConfigureGraph();

    void DisplayMessage(const QString& message);
    void DisplayWarning(const QString& warning);

public:
    explicit OscilloscopeWidget(QWidget *parent = nullptr);

    void UpdateGraph(const std::vector<ChannelSamples>& channel_samples_vec);
    void UpdateDevicePathList();
    void SetDevicePath(const QString& device_path);
};
