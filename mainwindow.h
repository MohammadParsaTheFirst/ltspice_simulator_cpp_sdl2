#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication> // for quiting the program
#include <QPixmap>
#include <QToolBar>
#include <QIcon>
#include "SchematicWidget.h"
#include "mainwindow.h"
#include "PlotWindow.h"
#include "ui_mainwindow.h"
#include "ChartWindow.h"
#include "TransientDialog.h"
#include "Circuit.h"
#include "TcpClient.h"
#include "TcpServer.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui;
    SchematicWidget* schematic;
    Circuit circuit;
    void setupWelcomeState();
    void setupSchematicState();
    ////////////////////////////////////////////////////////////////////////////////
    QAction* transientAction;          // Action for plotting tr. anal.
    QAction* networkAction;            // New Action for Networking

    TcpServer *tcpServer = nullptr;
    TcpClient *tcpClient = nullptr;
    //////////////////////////////////////////////////////////////////////////////////

    // Some items in menu bar to disable and enabling them
    QAction* settingsAction;
    QAction* newSchematicAction;
    QAction* openAction;
    QAction* configureAnalysisAction;
    QAction* runAction;
    QAction* wireAction;
    QAction* groundAction;
    QAction* voltageSourceAction;
    QAction* resistorAction;
    QAction* capacitorAction;
    QAction* inductorAction;
    QAction* diodeAction;
    QAction* nodeLibraryAction;
    QAction* labelAction;
    QAction* deleteModeAction;
    QAction* quitAction;

    private slots:
        void hNewSchematic();
        ///////////////////////////////////////////////////////////////////
        void openTransientDialog();
        void openNetworkDialog();
        void updateVoltageFromNetwork(double voltage);
        void updateNetworkStatus(const QString& msg);
        //////////////////////////////////////////////////////////////////
    void hShowSettings();

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

    void starterWindow();
    void initializeActions();
    void implementMenuBar();
    void implementToolBar();
    void shortcutRunner();
};


#endif //MAINWINDOW_H