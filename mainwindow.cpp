#include "mainwindow.h"
#include <QDir>

QString getSubcircuitLibraryPath() {
    QString appPath = QCoreApplication::applicationDirPath();
    QDir dir(appPath + "/lib");
    if (!dir.exists())
        dir.mkpath(".");
    return dir.path();
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    loadSubcircuitsFromLibrary();
    setWindowIcon(QIcon(":/icon.png"));
    this->resize(900, 600);
    schematicsPath = QCoreApplication::applicationDirPath() + "/Schematics";
    QDir dir(schematicsPath);
    if (!dir.exists())
        dir.mkpath(".");

    starterWindow();
    setupWelcomeState();
}

void MainWindow::loadSubcircuitsFromLibrary() {
    QString libraryPath = getSubcircuitLibraryPath();
    QDir directory(libraryPath);
    QStringList files = directory.entryList(QStringList() << "*.sub", QDir::Files);

    for (const QString& filename : files) {
        QString filePath = libraryPath + "/" + filename;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("Could not open subcircuit file: %s", qPrintable(filePath));
            continue;
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_5);
        SubcircuitDefinition subDef;
        in >> subDef;
        file.close();

        circuit.subcircuitDefinitions[subDef.name] = subDef;
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupWelcomeState() {
    this->setWindowTitle("ParsaSpice Simulator");

    if (centralWidget()) {
        delete centralWidget();
        schematic = nullptr;
    }

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
    subcircuitLibraryAction->setEnabled(false);
}

void MainWindow::setupSchematicState(const QString& projectName) {
    if (centralWidget() && schematic) {
        delete centralWidget();
        schematic = nullptr;
    }

    this->setWindowTitle(projectName);
    schematic = new SchematicWidget(&circuit, this);
    setCentralWidget(schematic);

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
    connect(createSubcircuitAction, &QAction::triggered, schematic, &SchematicWidget::startCreateSubcircuit);
    connect(subcircuitLibraryAction, &QAction::triggered, schematic, &SchematicWidget::startOpeningSubcircuitLibrary);

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
    createSubcircuitAction->setEnabled(true);
    subcircuitLibraryAction->setEnabled(true);
}

void MainWindow::hShowSettings() {
    QMessageBox::information(this, "Settings", "Buy premium!");
}

void MainWindow::hNewSchematic() {
    bool ok;
    QString projectName = QInputDialog::getText(this, "New Project", "Enter project name:", QLineEdit::Normal, "", &ok);
    if (ok && !projectName.isEmpty()) {
        circuit.clearSchematic();
        currentProjectPath.clear();
        currentProjectName = projectName;
        setupSchematicState("ParsaSpice - " + projectName);
    }
}

void MainWindow::hSaveProject() {
    QString filePath = currentProjectPath;
    if (filePath.isEmpty()) {
        QString projectFolderPath = schematicsPath + "/" + currentProjectName;
        QDir().mkpath(projectFolderPath);
        QString defaultPath = projectFolderPath + "/" + currentProjectName + ".psp";
        filePath = QFileDialog::getSaveFileName(this, "Save Schematic", defaultPath, "ParsaSpice Project (*.psp)");
        if (filePath.isEmpty())
            return;
        currentProjectPath = filePath;
    }

    try {
        circuit.saveToFile(currentProjectPath);
        QFileInfo fileInfo(currentProjectPath);
        setWindowTitle("ParsaSpice - " + fileInfo.fileName());
        QMessageBox::information(this, "Success", "Project saved successfully.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to save project: %1").arg(e.what()));
    }
}

void MainWindow::hOpenProject() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Schematic", schematicsPath, "ParsaSpice Project (*.psp)");
    if (filePath.isEmpty())
        return;

    try {
        circuit.loadFromFile(filePath);
        currentProjectPath = filePath;
        QFileInfo fileInfo(filePath);
        currentProjectName = fileInfo.baseName();
        setupSchematicState("ParsaSpice - " + fileInfo.fileName());
        schematic->update();
        QMessageBox::information(this, "Success", "Project loaded successfully.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load project: %1").arg(e.what()));
        circuit.clearSchematic();
        setupWelcomeState();
    }
}

void MainWindow::starterWindow() {
    initializeActions();

    connect(newSchematicAction, &QAction::triggered, this, &MainWindow::hNewSchematic);
    connect(openAction, &QAction::triggered, this, &MainWindow::hOpenProject);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::hShowSettings);
    connect(saveAction, &QAction::triggered, this, &MainWindow::hSaveProject);

    shortcutRunner();
    implementMenuBar();
    implementToolBar();
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
    subcircuitLibraryAction = new QAction("Open Subcircuit Library", this);
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
    hierarchy->addAction(createSubcircuitAction);
    hierarchy->addAction(subcircuitLibraryAction);

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