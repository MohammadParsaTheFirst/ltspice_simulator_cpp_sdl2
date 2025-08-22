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
#include <QFileDialog>
#include <QInputDialog>
#include "ui_mainwindow.h"
#include "SchematicWidget.h"
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
    QString currentProjectPath;
    QString schematicsPath;
    QString currentProjectName;

    void setupWelcomeState();
    void setupSchematicState(const QString& projectName = "Draft.asc");

    // Some items in menu bar to disable and enabling them
    QAction* settingsAction;
    QAction* newSchematicAction;
    QAction* saveAction;
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
    QAction* createSubcircuitAction;
    QAction* subcircuitLibraryAction;
    QAction* quitAction;

private slots:
    void hNewSchematic();
    void hShowSettings();
    void hSaveProject();
    void hOpenProject();

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

    void starterWindow();
    void initializeActions();
    void implementMenuBar();
    void implementToolBar();
    void shortcutRunner();

    void loadSubcircuitsFromLibrary();
};


#endif //MAINWINDOW_H
