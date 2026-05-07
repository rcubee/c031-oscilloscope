#include <QtWidgets>
#include <QApplication>
#include <QMainWindow>
#include "oscilloscope_widget.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    OscilloscopeWidget oscilloscope_widget = OscilloscopeWidget(&window);

    window.setWindowTitle("Oscilloscope");
    window.setCentralWidget(&oscilloscope_widget);
    window.resize(2560 / 2, 1440 / 2);
    window.show();

    return app.exec();
}
