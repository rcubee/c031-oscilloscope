#include "oscilloscope_widget.hpp"
#include <QDirIterator>
#include <QListView>
#include <QMessageBox>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "oscilloscope.hpp"
#include <QThread>

void OscilloscopeWidget::OnConnectButtonReleased()
{
    if (serial_connection.IsOpen()) {
        serial_connection.FrameStopCollectingSamples();

        serial_connection.Close();

        OnSerialConnectionClosed();
    }
    else {
        const QString device_path = ui.devicePath->text();

        if (device_path.isEmpty()) {
            DisplayWarning("Empty device path.");
            return;
        }

        serial_connection.setBaudRate(ui.baud_rate_combo_box->GetBaudRate());

        if (serial_connection.Open(device_path)) {
            OnSerialConnectionOpened();
        }
    }
}

void OscilloscopeWidget::OnSerialConnectionErrorOccured(QSerialPort::SerialPortError serial_port_error)
{
    if (serial_port_error == QSerialPort::SerialPortError::NoError) {
        return;
    }

    const QString warning = ui.devicePath->text() + ": " + serial_connection.errorString();

    serial_connection.Close();
    OnSerialConnectionClosed();

    DisplayWarning(warning);
}

// TODO: REname to pressed
void OscilloscopeWidget::OnChannelButtonToggled(ChannelButton* channel_button, EChannel channel, bool enabled)
{
    if (!serial_connection.IsOpen()) {
        channel_button->SetDisabled();

        DisplayWarning("Device is not connected.");
        return;
    }

    Channel& ch = channel == EChannel::_1 ? channel1 : channel2;

    if (enabled) {
        ch.SetEnabled();
    } else {
        ch.SetDisabled();
    }

    UpdateConfig();
}

void OscilloscopeWidget::OnSerialConnectionOpened()
{
    qDebug() << "Opened " << serial_connection.portName();

    serial_connection.FrameGetConfig();

    ui.connectButton->setText("Close");
    ui.devicePath->setReadOnly(true);
    ui.devicePathList->setEnabled(false);
    ui.baud_rate_combo_box->setDisabled(true);
}

void OscilloscopeWidget::OnFrameMessage(const std::string& message)
{
    DisplayMessage(QString::fromStdString(message));
}

void OscilloscopeWidget::OnSerialConnectionGetConfig(osc_frame_config config)
{
    qDebug() << "OnSerialConnectionGetConfig";

    if (config.channels_enabled & OSC_ECHANNEL_1) {
        channel1.SetEnabled();
        ui.ch1_button->SetEnabled();
    }

    if (config.channels_enabled & OSC_ECHANNEL_2) {
        channel2.SetEnabled();
        ui.ch2_button->SetEnabled();
    }

    ui.horizontal_dial->SetHorizontalScale(config.horizontal_scale);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    serial_connection.FrameStartCollectingSamples();
}

void OscilloscopeWidget::OnSerialConnectionClosed()
{
    qDebug() << "Closed " << serial_connection.portName();

    // Connection
    ui.connectButton->setText("Connect");
    ui.devicePath->setReadOnly(false);
    ui.devicePathList->setEnabled(true);
    ui.baud_rate_combo_box->setEnabled(true);

    // Vertical
    ui.ch1_button->SetDisabled();
    ui.ch2_button->SetDisabled();

    for (auto& abstract_series : ui.graph->chart()->series()) {
        auto line_series = static_cast<QLineSeries*>(abstract_series);

        line_series->clear();
    }

    UpdateDevicePathList();
}

void OscilloscopeWidget::OnVerticalDialChanged(int)
{
    channel1.SetVerticalScale(ui.ch1_dial->GetVerticalScale());
    channel2.SetVerticalScale(ui.ch2_dial->GetVerticalScale());
}

void OscilloscopeWidget::UpdateConfig()
{
    osc_echannel channels_enabled = 0;
    if (channel1.IsEnabled()) {
        channels_enabled |= OSC_ECHANNEL_1;
    }
    if (channel2.IsEnabled()) {
        channels_enabled |= OSC_ECHANNEL_2;
    }

    serial_connection.FrameConfig(channels_enabled, ui.horizontal_dial->GetHorizontalScale());

    x_axis->setRange(0, ui.horizontal_dial->GetHorizontalScale() * 10);
}

void OscilloscopeWidget::OnHorizontalScaleChanged(int _)
{
    UpdateConfig();
}

void OscilloscopeWidget::DisplayMessage(const QString& message)
{
    QMessageBox::information(this, "Information", message);
}

void OscilloscopeWidget::DisplayWarning(const QString& warning)
{
    QMessageBox::warning(this, "Warning", warning);
}

OscilloscopeWidget::OscilloscopeWidget(QWidget *parent)
    : QWidget(parent)
    , x_axis(new QValueAxis)
    , y_axis(new QValueAxis)
    , chart(new QChart)
    , channels({ Channel(EChannel::_1), Channel(EChannel::_2) })
    , channel1(channels.at(0))
    , channel2(channels.at(1))
{
    ui.setupUi(this);

    ConfigureGraph();

    serial_connection.Configure(
        QSerialPort::DataBits::Data8,
        QSerialPort::Parity::NoParity,
        QSerialPort::StopBits::OneStop,
        QSerialPort::BaudRate(BAUD_RATE_DEFAULT)
    );

    /* Connection & connection settings */
    connect(&serial_connection, &SerialConnection::SignalFrameMessage, this, &OscilloscopeWidget::OnFrameMessage);
    connect(&serial_connection, &SerialConnection::SignalFrameSamples, this, &OscilloscopeWidget::UpdateGraph);
    connect(&serial_connection, &SerialConnection::SignalFrameConfig, this, &OscilloscopeWidget::OnSerialConnectionGetConfig);
    connect(&serial_connection, &QSerialPort::errorOccurred, this, &OscilloscopeWidget::OnSerialConnectionErrorOccured);

    ui.devicePathList->setMaxCount(16);
    ui.devicePathList->setMaxVisibleItems(16);
    UpdateDevicePathList();
    ui.devicePath->setText(ui.devicePathList->currentText());

    connect(ui.connectButton, &QPushButton::released, this, &OscilloscopeWidget::OnConnectButtonReleased);
    connect(ui.devicePathList, &QComboBox::textActivated, this, &OscilloscopeWidget::SetDevicePath);

    /* Vertical */

    /* Channel 1 */
    // Position dial
    ui.ch1_pos_dial->SetLabel(ui.ch1_pos_dial_label);

    // Button
    ui.ch1_button->SetChannel(EChannel::_1);
    connect(ui.ch1_button, &ChannelButton::Toggled, this, &OscilloscopeWidget::OnChannelButtonToggled);

    // Scale dial
    ui.ch1_dial->SetLabel(ui.ch1_dial_label);
    connect(ui.ch1_dial, &QDial::valueChanged, this, &OscilloscopeWidget::OnVerticalDialChanged);

    /* Channel 2 */
    // Position dial
    ui.ch2_pos_dial->SetLabel(ui.ch2_pos_dial_label);

    // Button
    ui.ch2_button->SetChannel(EChannel::_2);
    connect(ui.ch2_button, &ChannelButton::Toggled, this, &OscilloscopeWidget::OnChannelButtonToggled);

    // Scale dial
    ui.ch2_dial->SetLabel(ui.ch2_dial_label);
    connect(ui.ch2_dial, &QDial::valueChanged, this, &OscilloscopeWidget::OnVerticalDialChanged);

    /* Horizontal */

    ui.horizontal_dial->SetLabel(ui.horizontal_dial_label);
    connect(ui.horizontal_dial, &QDial::valueChanged, this, &OscilloscopeWidget::OnHorizontalScaleChanged);

    /* Trigger */
}

void OscilloscopeWidget::ConfigureGraph()
{
    x_axis->setTitleText("Samples");
    x_axis->setLabelFormat("%ius");
    x_axis->setRange(0.0f, 1000.0f);
    x_axis->setTickCount(10 + 1);
    x_axis->setGridLinePen(QColor(127, 127, 127));

    y_axis->setTitleText("Voltage");
    // y_axis->setLabelFormat("%.02fV");
    y_axis->setLabelFormat(" ");
    // y_axis->setRange(-3.3f, 3.3f);
    y_axis->setRange(-4.0f, 4.0f);
    y_axis->setTickCount(8 + 1);
    y_axis->setGridLineColor(QColor(127, 127, 127));

    chart->addSeries(channel1.GetSeries());
    chart->addSeries(channel2.GetSeries());
    chart->addAxis(x_axis, Qt::AlignBottom);
    chart->addAxis(y_axis, Qt::AlignLeft);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setPlotAreaBackgroundBrush(QBrush(Qt::black));
    chart->setPlotAreaBackgroundVisible();

    channel1.AttachAxis(x_axis, y_axis);
    channel2.AttachAxis(x_axis, y_axis);

    // ui.graph->setRubberBand(QChartView::HorizontalRubberBand);
    // ui.graph->setInteractive(true);
    ui.graph->setBackgroundRole(QPalette::ColorRole::Window);
    ui.graph->setForegroundRole(QPalette::ColorRole::Window);
    ui.graph->setPalette(QApplication::palette());
    ui.graph->setRenderHint(QPainter::Antialiasing);
    ui.graph->setChart(chart);
}


void OscilloscopeWidget::UpdateGraph(const std::vector<ChannelSamples>& channel_samples_vec)
{
    for (auto& channel_samples : channel_samples_vec) {
        if (channel_samples.channel == EChannel::_1) {
            channel1.Replace(channel_samples.samples, ui.ch1_pos_dial->GetPosition());
        }
        else if (channel_samples.channel == EChannel::_2) {
            channel2.Replace(channel_samples.samples, ui.ch2_pos_dial->GetPosition());
        }
    }
}

void OscilloscopeWidget::UpdateDevicePathList()
{
    QDirIterator dir_iterator("/dev/", QDir::Filter::System);
    QStringList device_list;

    while (dir_iterator.hasNext()) {
        dir_iterator.next();

        if (dir_iterator.fileName().startsWith("ttyACM")) {
            device_list.append(dir_iterator.filePath());
        }
    }

    ui.devicePathList->clear();
    ui.devicePathList->addItems(device_list);
}

void OscilloscopeWidget::SetDevicePath(const QString& device_path)
{
    ui.devicePath->setText(device_path);
}
