#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    register_polymorphic_types();
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}