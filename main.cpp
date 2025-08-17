#include <QApplication>
#include "mainwindow.h"
#include <cereal/archives/binary.hpp>
#include <cereal/details/static_object.hpp>

int main(int argc, char *argv[])
{
    cereal::detail::dynamic_init::waveforms();

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}