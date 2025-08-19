#include "mainwindow.h"

#include "NetworkDialog.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowTitle("LTspice");
    this->resize(900, 600);
    starterWindow();
    setupWelcomeState();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupWelcomeState() {
    this->setWindowTitle("LTspice Simulator");

    QLabel* backgroundLabel = new QLabel(this);
    QPixmap backgroundImage(":/background.jpg");
    backgroundLabel->setPixmap(backgroundImage);
    backgroundLabel->setScaledContents(true);
    setCentralWidget(backgroundLabel);

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
}

void MainWindow::setupSchematicState() {
    this->setWindowTitle("LTspice - Draft.asc");
    schematic = new SchematicWidget(&circuit, this);
    setCentralWidget(schematic);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // This is the new connection to our dialog handler
    connect(runAction, &QAction::triggered, this, &MainWindow::openTransientDialog);
    // Connect the new network action
    connect(networkAction, &QAction::triggered, this, &MainWindow::openNetworkDialog);
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //connect(runAction, &QAction::triggered, schematic, &SchematicWidget::startRunAnalysis);
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

    /////////////////////////////////////////////////////////////////////////////////////////
    // Enable network action when in schematic state
    networkAction->setEnabled(true);
    ////////////////////////////////////////////////////////////////////////////////////////
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
    setupSchematicState();
}

void MainWindow::starterWindow() {
    initializeActions();

    connect(newSchematicAction, &QAction::triggered, this, &MainWindow::hNewSchematic);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::hShowSettings);

    shortcutRunner();
    implementMenuBar();
    implementToolBar();
}

void MainWindow::initializeActions() {
    settingsAction = new QAction(QIcon(":/icon/icons/settings.png"), "Settings", this);
    newSchematicAction = new QAction(QIcon(":/icon/icons/newSchematic.png"), "New Schematic (CTRL+N)", this);
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
    quitAction = new QAction("Exit", this);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    networkAction = new QAction(QIcon(":/icon/icons/network.png"), "Network (N)", this); // New network action
    //////////////////////////////////////////////////////////////////////////////////////////////////////
}

void MainWindow::implementMenuBar() {
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(newSchematicAction);
    file->addAction(openAction);
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

    QMenu* Hierarchy = menuBar()->addMenu(tr("&Hierarchy"));

    QMenu* view = menuBar()->addMenu(tr("&View"));

    QMenu* simulate = menuBar()->addMenu(tr("&Simulate"));
    simulate->addAction(runAction);
    simulate->addSeparator();
    simulate->addAction(settingsAction);
    simulate->addSeparator();
    simulate->addAction(configureAnalysisAction);

    QMenu* tools = menuBar()->addMenu(tr("&Tools"));
    tools->addAction(settingsAction);
    ///////////////////////////////////////////////////////////////////////////////////////////
    tools->addAction(networkAction); // Add network action to Tools menu
    ///////////////////////////////////////////////////////////////////////////////////////////

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
    //////////////////////////////////////////////////////////////////////////////
    mainToolBar->addAction(networkAction); // Add network action to toolbar
    //////////////////////////////////////////////////////////////////////////////

    mainToolBar->setIconSize(QSize(40, 40));
}

void MainWindow::shortcutRunner() {
    newSchematicAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
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
    //////////////////////////////////////////////////////////////////////////////////
    networkAction->setShortcut(QKeySequence(Qt::Key_N)); // New shortcut for network
    //////////////////////////////////////////////////////////////////////////////////
}


void MainWindow::openTransientDialog() {
    TransientDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double startTime = dialog.getStartTime();
        double stopTime = dialog.getStopTime();
        double stepTime = dialog.getStepTime();
        QString parameter = dialog.getParameter();

        if (stepTime <= 0) {
            QMessageBox::warning(this, "Input Error", "Step time must be greater than zero.");
            return;
        }

        // Call your existing transient analysis function to run the simulation and store results
        circuit.runTransientAnalysis(stopTime, startTime, stepTime);

        // Get the specific data to plot using the new function
        std::pair<std::string, std::vector<double>> results = circuit.getTransientResults(parameter.toStdString());

        // Check if there's any data to plot
        if (!results.second.empty()) {
            // Create and show the new plot window
            PlotWindow *plotWindow = new PlotWindow(this);
            std::vector<double> timePoints;
            for (double t = startTime; t <= stopTime; t += stepTime) {
                timePoints.push_back(t);
            }
            plotWindow->plotData(timePoints, results.second, QString::fromStdString(results.first));
            plotWindow->show();
        } else {
            QMessageBox::warning(this, "Analysis Failed", "Could not generate plot data. Please check your circuit and parameters.");
        }
    }
}

void MainWindow::openNetworkDialog() {
    if (tcpServer || tcpClient) {
        QMessageBox::warning(this, "Network Error", "A network connection is already active. Please restart the application to change settings.");
        return;
    }

    NetworkDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        int port = dialog.getPort();
        if (dialog.isServerMode()) {
            // Server mode
            tcpServer = new TcpServer(this);
            connect(tcpServer, &TcpServer::logMessage, this, &MainWindow::updateNetworkStatus);
            tcpServer->startServer(port);
            QMessageBox::information(this, "Server Mode", "Server started. Place a wireless voltage source to begin.");
        } else {
            // Client mode
            QString ipAddress = dialog.getIpAddress();
            tcpClient = new TcpClient(this);
            connect(tcpClient, &TcpClient::logMessage, this, &MainWindow::updateNetworkStatus);
            connect(tcpClient, &TcpClient::voltageReceived, this, &MainWindow::updateVoltageFromNetwork);
            tcpClient->connectToServer(ipAddress, port);
            QMessageBox::information(this, "Client Mode", "Client attempting to connect. Place a wireless voltage source to receive data.");
        }
    }
}

void MainWindow::updateVoltageFromNetwork(double voltage) {
    // This is a placeholder. You'll need to update your WirelessVoltageSource component here.
    // For example, you might have a method in your Circuit or SchematicWidget to set the voltage of a specific component.
    // circuit.setWirelessSourceVoltage(voltage);
    qDebug() << "Received voltage from network: " << voltage;
}

void MainWindow::updateNetworkStatus(const QString& msg) {
    // This is a placeholder for a status bar or log in your main window to display network messages.
    // For now, it will print to the console.
    qDebug() << msg;
}