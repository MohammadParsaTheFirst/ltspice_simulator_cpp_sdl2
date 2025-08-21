#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), circuit() {
    schematic = new SchematicWidget(&circuit, this);
    setCentralWidget(schematic);
    schematic->setMinimumSize(800, 600);

    qDebug() << "MainWindow constructor: circuit=" << &circuit
             << " schematic=" << schematic
             << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);

    ui->setupUi(this);
    qDebug() << "After ui->setupUi: schematic=" << schematic
             << " centralWidget=" << centralWidget()
             << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);

    if (centralWidget() != schematic) {
        qDebug() << "WARNING: ui->setupUi changed centralWidget to" << centralWidget();
        delete centralWidget(); // Delete the default widget to avoid memory leak
        setCentralWidget(schematic); // Restore schematic
        schematic->show();
    }

    this->setWindowTitle("LTspice");
    this->resize(900, 600);

    // Temporarily skip to avoid crash
    starterWindow();
    setupWelcomeState();
    qDebug() << "MainWindow constructor end: circuit=" << &circuit
             << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
}
// MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
//     // Initialize schematic first
//     schematic = new SchematicWidget(&circuit, this);
//     setCentralWidget(schematic);
//
//     schematic->setMinimumSize(800, 600);
//     qDebug() << "MainWindow constructor: circuit=" << &circuit
//              << " schematic=" << schematic
//              << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
//
//     // Debug after initialization
//     qDebug() << "MainWindow constructor: circuit=" << &circuit
//              << " schematic=" << schematic
//              << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
//
//     ui->setupUi(this);
//     qDebug() << "After ui->setupUi: schematic=" << schematic
//              << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
//
//     this->setWindowTitle("LTspice");
//     this->resize(900, 600);
//     starterWindow();
//     setupWelcomeState();
//
//
//     qDebug() << "MainWindow constructor: circuit=" << &circuit << " schematic->circuit_ptr=" << schematic->getCircuitPtr();
// }

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupWelcomeState() {
    qDebug() << "setupWelcomeState: Start, schematic=" << schematic
             << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);

    // Avoid: setCentralWidget(new QWidget);
    // Instead, show schematic if hidden
    if (schematic) {
        schematic->show();
        schematic->setCircuitPtr(&circuit);
    }

    qDebug() << "setupWelcomeState: End, schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);

    this->setWindowTitle("LTspice Simulator");

    QLabel* backgroundLabel = new QLabel(this);
    QPixmap backgroundImage(":/background.jpg");
    backgroundLabel->setPixmap(backgroundImage);
    backgroundLabel->setScaledContents(true);
    setCentralWidget(backgroundLabel);

    saveAction->setEnabled(false);
    configureAnalysisAction->setEnabled(false);
    runAction->setEnabled(false);
    wireAction->setEnabled(false);
    groundAction->setEnabled(false);
    voltageSourceAction->setEnabled(false);
    resistorAction->setEnabled(false);
    capacitorAction->setEnabled(false);
    inductorAction->setEnabled(false);
    diodeAction->setEnabled(false);
    nodeLibraryAction->setEnabled(false);
    labelAction->setEnabled(false);
    deleteModeAction->setEnabled(false);
    createSubcircuitAction->setEnabled(false);

}

void MainWindow::setupSchematicState() {
    this->setWindowTitle("LTspice - Draft.asc");
    schematic = new SchematicWidget(&circuit, this);
    setCentralWidget(schematic);

    connect(saveAction, &QAction::triggered, this, &MainWindow::hSaveProject);
    connect(runAction, &QAction::triggered, schematic, &SchematicWidget::startRunAnalysis);
    connect(configureAnalysisAction, &QAction::triggered, schematic, &SchematicWidget::startOpenConfigureAnalysis);
    connect(wireAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingWire);
    connect(groundAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingGround);
    connect(resistorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingResistor);
    connect(capacitorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingCapacitor);
    connect(inductorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingInductor);
    connect(voltageSourceAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingVoltageSource);
    connect(diodeAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingDiode);
    connect(nodeLibraryAction, &QAction::triggered, schematic, &SchematicWidget::startOpenNodeLibrary);
    connect(labelAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingLabel);
    connect(deleteModeAction, &QAction::triggered, schematic, &SchematicWidget::startDeleteComponent);
    // connect(createSubcircuitAction, &QAction::triggered, schematic, &SchematicWidget::startCreateSubcircuit);

    saveAction->setEnabled(true);
    configureAnalysisAction->setEnabled(true);
    runAction->setEnabled(true);
    wireAction->setEnabled(true);
    groundAction->setEnabled(true);
    voltageSourceAction->setEnabled(true);
    resistorAction->setEnabled(true);
    capacitorAction->setEnabled(true);
    inductorAction->setEnabled(true);
    diodeAction->setEnabled(true);
    nodeLibraryAction->setEnabled(true);
    labelAction->setEnabled(true);
    deleteModeAction->setEnabled(true);
    // createSubcircuitAction->setEnabled(true);
}

// void MainWindow::openChartWindow()
// {
//     ChartWindow *chartWin = new ChartWindow();
//     chartWin->setAttribute(Qt::WA_DeleteOnClose);
//     chartWin->show();
// }

void MainWindow::hShowSettings() {
    QMessageBox::information(this, "Settings", "Buy premium!");
}

void MainWindow::hNewSchematic() {
    bool ok;
    QString projectName = QInputDialog::getText(this, "New Project", "Enter project name:", QLineEdit::Normal, "", &ok);
    if (ok && !projectName.isEmpty()) {
        circuit.newProject(projectName.toStdString());
        setupSchematicState();
        this->setWindowTitle("LTspice - " + projectName);
    }
}

void MainWindow::hSaveProject() {
    circuit.saveProject();
    QMessageBox::information(this, "Save Project", "Project saved.");
}

// void MainWindow::hOpenProject() {
//     QString schematicsDir = circuit.getProjectDirectory();
//     QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", schematicsDir);
//
//     if (!projectPath.isEmpty()) {
//         QDir dir(projectPath);
//         QString projectName = dir.dirName();
//         circuit.loadProject(projectName.toStdString());
//         if (!schematic) {
//             schematic = new SchematicWidget(&circuit, this);
//             setCentralWidget(schematic);
//             schematic->setMinimumSize(800, 600); // Ensure reasonable size
//             qDebug() << "Created new SchematicWidget with circuit_ptr";
//         } else {
//             schematic->setCircuitPtr(&circuit); // Ensure circuit_ptr is set
//             qDebug() << "Updated existing SchematicWidget with circuit_ptr";
//         }
//         schematic->reloadFromCircuit();
//         schematic->update(); // Force repaint
//         this->setWindowTitle("LTspice - " + projectName);
//         qDebug() << "After open: Components=" << circuit.getComponentGraphics().size()
//                  << " Wires=" << circuit.getWires().size()
//                  << " Grounds=" << circuit.getGrounds().size()
//                  << " Labels=" << circuit.getLabels().size();
//     }
// }
// -------------------------------------
// void MainWindow::hOpenProject() {
//     QString schematicsDir = circuit.getProjectDirectory();
//     QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", schematicsDir);
//
//     if (!projectPath.isEmpty()) {
//         QDir dir(projectPath);
//         QString projectName = dir.dirName();
//         qDebug() << "hOpenProject: Before loadProject: circuit=" << &circuit
//                  << " schematic=" << schematic
//                  << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
//
//         circuit.loadProject(projectName.toStdString());
//
//         if (!schematic) {
//             schematic = new SchematicWidget(&circuit, this);
//             setCentralWidget(schematic);
//             schematic->setMinimumSize(800, 600);
//             qDebug() << "Created new SchematicWidget: circuit_ptr=" << schematic->getCircuitPtr();
//         } else {
//             schematic->setCircuitPtr(&circuit);
//             qDebug() << "Updated SchematicWidget: circuit_ptr=" << schematic->getCircuitPtr();
//         }
//
//         qDebug() << "Before reloadFromCircuit: Components=" << circuit.getComponentGraphics().size()
//                  << " Wires=" << circuit.getWires().size()
//                  << " Grounds=" << circuit.getGrounds().size()
//                  << " Labels=" << circuit.getLabels().size();
//
//         if (schematic) {
//             schematic->reloadFromCircuit();
//             qDebug() << "After reloadFromCircuit: circuit_ptr=" << schematic->getCircuitPtr();
//             schematic->update();
//             qDebug() << "After update";
//         }
//
//         this->setWindowTitle("LTspice - " + projectName);
//         qDebug() << "After hOpenProject: circuit=" << &circuit
//                  << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
//     }
// }
void MainWindow::hOpenProject() {
    QString schematicsDir = circuit.getProjectDirectory();
    QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", schematicsDir);

    if (!projectPath.isEmpty()) {
        QDir dir(projectPath);
        QString projectName = dir.dirName();
        qDebug() << "hOpenProject: Before loadProject: circuit=" << &circuit
                 << " schematic=" << schematic
                 << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);

        circuit.loadProject(projectName.toStdString());

        if (!schematic) {
            schematic = new SchematicWidget(&circuit, this);
            setCentralWidget(schematic);
            schematic->setMinimumSize(800, 600);
            qDebug() << "Created new SchematicWidget: circuit_ptr=" << schematic->getCircuitPtr();
        } else {
            schematic->setCircuitPtr(&circuit);
            schematic->show();
            qDebug() << "Updated SchematicWidget: circuit_ptr=" << schematic->getCircuitPtr();
        }

        qDebug() << "Before reloadFromCircuit: Components=" << circuit.getComponentGraphics().size()
                 << " Wires=" << circuit.getWires().size()
                 << " Grounds=" << circuit.getGrounds().size()
                 << " Labels=" << circuit.getLabels().size();

        if (schematic) {
            schematic->reloadFromCircuit();
            qDebug() << "After reloadFromCircuit: circuit_ptr=" << schematic->getCircuitPtr();
            schematic->update();
            qDebug() << "After update";
        }

        this->setWindowTitle("LTspice - " + projectName);
        qDebug() << "After hOpenProject: circuit=" << &circuit
                 << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
    }
}
// void MainWindow::hOpenProject() {
//     QString schematicsDir = circuit.getProjectDirectory();
//     QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", schematicsDir);
//
//     if (!projectPath.isEmpty()) {
//         QDir dir(projectPath);
//         QString projectName = dir.dirName();
//         circuit.loadProject(projectName.toStdString());
//         if (!schematic)
//             setupSchematicState();
//         schematic->reloadFromCircuit();
//         this->setWindowTitle("LTspice - " + projectName);
//     }
// }

void MainWindow::starterWindow() {
    qDebug() << "starterWindow: Start, schematic=" << schematic
             << " schematic->circuit_ptr=" << (schematic ? schematic->getCircuitPtr() : nullptr);
    // Avoid resetting schematic or circuit_ptr
    initializeActions();

    connect(newSchematicAction, &QAction::triggered, this, &MainWindow::hNewSchematic);
    connect(openAction, &QAction::triggered, this, &MainWindow::hOpenProject);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::hShowSettings);

    shortcutRunner();
    implementMenuBar();
    implementToolBar();
    qDebug() << "starterWindow: End";
}

void MainWindow::initializeActions() {
    settingsAction = new QAction(QIcon(":/icon/icons/settings.png"), "Settings", this);
    newSchematicAction = new QAction(QIcon(":/icon/icons/newSchematic.png"), "New Schematic (CTRL+N)", this);
    saveAction = new QAction(QIcon(":/icon/icons/save.png"), "Save (CTRL+S)", this);
    openAction = new QAction(QIcon(":/icon/icons/open.png"), "Open (CTRL+O)", this);
    configureAnalysisAction = new QAction(QIcon(":/icon/icons/configureAnalysis.png"), "Configure Analysis (A)", this);
    runAction = new QAction(QIcon(":/icon/icons/run.png"), "Run (ALT+R)", this);
    wireAction = new QAction(QIcon(":/icon/icons/wire.png"), "Wire (W)", this);
    groundAction = new QAction(QIcon(":/icon/icons/ground.png"), "Ground (G)", this);
    voltageSourceAction = new QAction(QIcon(":/icon/icons/voltageSource.png"), "Voltage Source (V)", this);
    resistorAction = new QAction(QIcon(":/icon/icons/resistor.png"), "Resistor (R)", this);
    capacitorAction = new QAction(QIcon(":/icon/icons/Capacitor.png"), "Capacitor (C)", this);
    inductorAction = new QAction(QIcon(":/icon/icons/inductor.png"), "Inductor (L)", this);
    diodeAction = new QAction(QIcon(":/icon/icons/diode.png"), "Diode (D)", this);
    nodeLibraryAction = new QAction(QIcon(":/icon/icons/nodeLibrary.png"), "Node Library (P)", this);
    labelAction = new QAction(QIcon(":/icon/icons/text.png"), "Text (T)", this);
    deleteModeAction = new QAction(QIcon(":/icon/icons/deleteMode.png"), "Delete Mode (Backspace or Del)", this);
    createSubcircuitAction = new QAction("Create Subcircuit", this);
    quitAction = new QAction("Exit", this);
}

void MainWindow::implementMenuBar() {
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(newSchematicAction);
    file->addAction(openAction);
    file->addAction(saveAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu* edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(labelAction);
    edit->addAction(configureAnalysisAction);
    edit->addAction(resistorAction);
    edit->addAction(capacitorAction);
    edit->addAction(inductorAction);
    edit->addAction(diodeAction);
    edit->addAction(nodeLibraryAction);
    edit->addAction(wireAction);
    edit->addAction(groundAction);
    edit->addAction(deleteModeAction);
    edit->addAction(createSubcircuitAction);

    QMenu* hierarchy = menuBar()->addMenu(tr("&Hierarchy"));
    // hierarchy->addAction(createSubcircuitAction);

    QMenu* view = menuBar()->addMenu(tr("&View"));

    QMenu* simulate = menuBar()->addMenu(tr("&Simulate"));
    simulate->addAction(runAction);
    simulate->addSeparator();
    simulate->addAction(settingsAction);
    simulate->addSeparator();
    simulate->addAction(configureAnalysisAction);

    QMenu* tools = menuBar()->addMenu(tr("&Tools"));
    tools->addAction(settingsAction);

    QMenu* window = menuBar()->addMenu(tr("&Window"));

    QMenu* help = menuBar()->addMenu("&Help");
    help->addAction("About the program");
}

void MainWindow::implementToolBar() {
    QToolBar* mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setMovable(false);

    mainToolBar->addAction(settingsAction);
    mainToolBar->addAction(newSchematicAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addAction(configureAnalysisAction);
    mainToolBar->addAction(runAction);
    mainToolBar->addAction(wireAction);
    mainToolBar->addAction(groundAction);
    mainToolBar->addAction(voltageSourceAction);
    mainToolBar->addAction(resistorAction);
    mainToolBar->addAction(capacitorAction);
    mainToolBar->addAction(inductorAction);
    mainToolBar->addAction(diodeAction);
    mainToolBar->addAction(nodeLibraryAction);
    mainToolBar->addAction(labelAction);
    mainToolBar->addAction(deleteModeAction);

    mainToolBar->setIconSize(QSize(40, 40));
}

void MainWindow::shortcutRunner() {
    newSchematicAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    configureAnalysisAction->setShortcut(QKeySequence(Qt::Key_A));
    runAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_R));
    wireAction->setShortcut(QKeySequence(Qt::Key_W));
    groundAction->setShortcut(QKeySequence(Qt::Key_G));
    voltageSourceAction->setShortcut(QKeySequence(Qt::Key_V));
    resistorAction->setShortcut(QKeySequence(Qt::Key_R));
    capacitorAction->setShortcut(QKeySequence(Qt::Key_C));
    inductorAction->setShortcut(QKeySequence(Qt::Key_L));
    diodeAction->setShortcut(QKeySequence(Qt::Key_D));
    nodeLibraryAction->setShortcut(QKeySequence(Qt::Key_P));
    labelAction->setShortcut(QKeySequence(Qt::Key_T));
    deleteModeAction->setShortcuts({QKeySequence(Qt::Key_Backspace), QKeySequence(Qt::Key_Delete)});
}