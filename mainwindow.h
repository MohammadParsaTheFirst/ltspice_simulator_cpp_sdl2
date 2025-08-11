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
#include "ui_mainwindow.h"
#include "ChartWindow.h"
#include "Circuit.h"


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
    QAction* textAction;
    QAction* deleteModeAction;
    QAction* quitAction;

private slots:
    void hNewSchematic();
    void hShowSettings();

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

    void starterWindow();
    void initializeActions();
    void implementMenuBar();
    void implementToolBar();
};


#endif //MAINWINDOW_H
