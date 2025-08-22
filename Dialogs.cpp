#include "Dialogs.h"


ValueDialog::ValueDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Enter component value");

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Value (e.g., 1k, 10u, 1000)", this);
    valueEdit = new QLineEdit(this);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    layout->addWidget(label);
    layout->addWidget(valueEdit);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ValueDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ValueDialog::reject);

    valueEdit->setFocus();
}

QString ValueDialog::getValue() const {
    return valueEdit->text();
}


SourceValueDialog::SourceValueDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Enter source value");
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* typeGroupBox = new QGroupBox("Source type", this);
    QHBoxLayout* typeGroupBoxLayout = new QHBoxLayout();
    dcForm = new QRadioButton("DC", this);
    sinForm = new QRadioButton("Sinusoidal", this);
    dcForm->setChecked(true);
    typeGroupBoxLayout->addWidget(dcForm);
    typeGroupBoxLayout->addWidget(sinForm);
    typeGroupBox->setLayout(typeGroupBoxLayout);
    layout->addWidget(typeGroupBox);

    dcGroupBox = new QGroupBox("DC parameters", this);
    QFormLayout* dcFormLayout = new QFormLayout();
    dcInput = new QLineEdit(this);
    dcFormLayout->addRow("Value:", dcInput);
    dcGroupBox->setLayout(dcFormLayout);
    layout->addWidget(dcGroupBox);

    sinGroupBox = new QGroupBox("Sinusoidal parameters", this);
    QFormLayout* sinFormLayout = new QFormLayout();
    sinOffset = new QLineEdit(this);
    sinAmplitude = new QLineEdit( this);
    sinFrequency = new QLineEdit(this);
    sinFormLayout->addRow("DC Offset:", sinOffset);
    sinFormLayout->addRow("Amplitude:", sinAmplitude);
    sinFormLayout->addRow("Frequency:", sinFrequency);
    sinGroupBox->setLayout(sinFormLayout);
    layout->addWidget(sinGroupBox);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ValueDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ValueDialog::reject);
    connect(sinForm, &QRadioButton::toggled, this, &SourceValueDialog::showSinOrNot);

    showSinOrNot(false);
}

void SourceValueDialog::showSinOrNot(bool isSinusoidal) {
    sinGroupBox->setEnabled(isSinusoidal);
    dcGroupBox->setEnabled(!isSinusoidal);
}


bool SourceValueDialog::isSinusoidal() const {
    return sinForm->isChecked();
}

QString SourceValueDialog::getDCValue() const { return dcInput->text(); }
QString SourceValueDialog::getSinOffset() const { return sinOffset->text(); }
QString SourceValueDialog::getSinAmplitude() const { return sinAmplitude->text(); }
QString SourceValueDialog::getSinFrequency() const { return sinFrequency->text(); }


NodeLibraryDialog::NodeLibraryDialog(Circuit* circuit, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Node library");
    setMinimumSize(300,400);

    listWidget = new QListWidget(this);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &NodeLibraryDialog::doubleClickedOnItem);

    QListWidgetItem* resistorItem = new QListWidgetItem("Resistor");
    QListWidgetItem* capacitorItem = new QListWidgetItem("Capacitor");
    QListWidgetItem* inductorItem = new QListWidgetItem("Inductor");
    QListWidgetItem* diodeItem = new QListWidgetItem("Diode");
    QListWidgetItem* voltageSourceItem = new QListWidgetItem("Independent voltage source");
    QListWidgetItem* acVoltageSourceItem = new QListWidgetItem("AC Voltage");
    QListWidgetItem* currentSourceItem = new QListWidgetItem("Independent current source");
    QListWidgetItem* vcvsItem = new QListWidgetItem("Voltage dependent voltage source");
    QListWidgetItem* vccsItem = new QListWidgetItem("Voltage dependent current source");
    QListWidgetItem* ccvsItem = new QListWidgetItem("Current dependent voltage source");
    QListWidgetItem* cccsItem = new QListWidgetItem("Current dependent current source");

    resistorItem->setData(Qt::UserRole, "R");
    capacitorItem->setData(Qt::UserRole, "C");
    inductorItem->setData(Qt::UserRole, "L");
    diodeItem->setData(Qt::UserRole, "D");
    voltageSourceItem->setData(Qt::UserRole, "V");
    acVoltageSourceItem->setData(Qt::UserRole, "AC");
    currentSourceItem->setData(Qt::UserRole, "I");
    vcvsItem->setData(Qt::UserRole, "E");
    vccsItem->setData(Qt::UserRole, "G");
    ccvsItem->setData(Qt::UserRole, "H");
    cccsItem->setData(Qt::UserRole, "F");

    listWidget->addItem(resistorItem);
    listWidget->addItem(capacitorItem);
    listWidget->addItem(inductorItem);
    listWidget->addItem(diodeItem);
    listWidget->addItem(voltageSourceItem);
    listWidget->addItem(acVoltageSourceItem);
    listWidget->addItem(currentSourceItem);
    listWidget->addItem(vcvsItem);
    listWidget->addItem(vccsItem);
    listWidget->addItem(ccvsItem);
    listWidget->addItem(cccsItem);

    QListWidgetItem* separator = new QListWidgetItem("-------------- Subcircuits --------------");
    separator->setFlags(separator->flags() & ~Qt::ItemIsSelectable);
    listWidget->addItem(separator);
    if (circuit) {
        for (const auto& pair: circuit->subcircuitDefinitions) {
            QString subcircuitName = QString::fromStdString(pair.first);
            QListWidgetItem* subcircuitItem = new QListWidgetItem(subcircuitName);
            subcircuitItem->setData(Qt::UserRole, "U:" + subcircuitName);
            listWidget->addItem(subcircuitItem);
        }
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
}

void NodeLibraryDialog::doubleClickedOnItem(QListWidgetItem* item) {
    QString componentType = item->data(Qt::UserRole).toString();
    emit componentSelected(componentType);
    accept();
}


LabelDialog::LabelDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Node Label");
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Enter node label:", this);
    labelLineEdit = new QLineEdit(this);
    labelButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(label);
    layout->addWidget(labelLineEdit);
    layout->addWidget(labelButtonBox);
    connect(labelButtonBox, &QDialogButtonBox::accepted, this, &LabelDialog::accept);
    connect(labelButtonBox, &QDialogButtonBox::rejected, this, &LabelDialog::reject);
    labelLineEdit->setFocus();
}

QString LabelDialog::getLabel() const {
    return labelLineEdit->text();
}


ConfigureAnalysisDialog::ConfigureAnalysisDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Configure Analysis");
    tabWidget = new QTabWidget(this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // Transient Tab
    QWidget* transientTab = new QWidget(this);
    QFormLayout* transientLayout = new QFormLayout(transientTab);
    tStopEdit = new QLineEdit(this);
    tStartEdit = new QLineEdit(this);
    tStepEdit = new QLineEdit(this);
    transientParameterEdit = new QLineEdit(this);
    transientLayout->addRow(new QLabel("Stop time:"), tStopEdit);
    transientLayout->addRow(new QLabel("Time to start saving data:"), tStartEdit);
    transientLayout->addRow(new QLabel("Maximum Timestep:"), tStepEdit);
    transientLayout->addRow(new QLabel("Parameter (e.g. V(N_1_1), I(R1)):"), transientParameterEdit);
    tabWidget->addTab(transientTab, "Transient");

    // AC Sweep Tab
    QWidget* ACSweepTab = new QWidget(this);
    QFormLayout* acSweepLayout = new QFormLayout(ACSweepTab);
    typeOfSweepComboBox = new QComboBox(ACSweepTab);
    typeOfSweepComboBox->addItem("Octave");
    typeOfSweepComboBox->addItem("Decade");
    typeOfSweepComboBox->addItem("Linear");
    ACOmegaStart = new QLineEdit(this);
    ACOmegaStop = new QLineEdit(this);
    ACNPoint = new QLineEdit(this);
    ACSweepParameterEdit = new QLineEdit(this);
    acSweepLayout->addRow(new QLabel("Start frequency:"), ACOmegaStart);
    acSweepLayout->addRow(new QLabel("Stop frequency:"), ACOmegaStop);
    acSweepLayout->addRow(new QLabel("Number of points:"), ACNPoint);
    acSweepLayout->addRow(new QLabel("Type of sweep:"), typeOfSweepComboBox);
    acSweepLayout->addRow(new QLabel("Parameter (e.g. V(N_1_1), I(R1)):"), ACSweepParameterEdit);
    tabWidget->addTab(ACSweepTab, "AC Analysis");

    // Phase Sweep
    QWidget* phaseSweepTab = new QWidget(this);
    QFormLayout* phaseSweepLayout = new QFormLayout(phaseSweepTab);
    phaseBaseFrequency = new QLineEdit(this);
    phaseStart = new QLineEdit(this);
    phaseStop = new QLineEdit(this);
    phaseNPoints = new QLineEdit(this);
    phaseParameterEdit = new QLineEdit(this);
    phaseSweepLayout->addRow(new QLabel("Base frequency:"), phaseBaseFrequency);
    phaseSweepLayout->addRow(new QLabel("Start phase:"), phaseStart);
    phaseSweepLayout->addRow(new QLabel("Stop phase:"), phaseStop);
    phaseSweepLayout->addRow(new QLabel("Number of points:"), phaseNPoints);
    phaseSweepLayout->addRow(new QLabel("Parameter (e.g. V(N_1_1), I(R1)):"), phaseParameterEdit);
    phaseSweepTab->setEnabled(false);
    tabWidget->addTab(phaseSweepTab, "Phase Sweep");

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigureAnalysisDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigureAnalysisDialog::reject);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
}

int ConfigureAnalysisDialog::getSelectedAnalysisType() const {return tabWidget->currentIndex();}
QString ConfigureAnalysisDialog::getTransientTstop() const {return tStopEdit->text();}
QString ConfigureAnalysisDialog::getTransientTstart() const {return tStartEdit->text();}
QString ConfigureAnalysisDialog::getTransientTstep() const {return tStepEdit->text();}
QString ConfigureAnalysisDialog::getTransientParameter() const {return transientParameterEdit->text();}
QString ConfigureAnalysisDialog::getACOmegaStart() const {return ACOmegaStart->text();}
QString ConfigureAnalysisDialog::getACOmegaStop() const {return ACOmegaStop->text();}
QString ConfigureAnalysisDialog::getACNPoints() const {return ACNPoint->text();}
QString ConfigureAnalysisDialog::getACParameter() const {return ACSweepParameterEdit->text();}


SubcircuitLibarary::SubcircuitLibarary(Circuit* circuit, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Subcircuit Library");
    setMinimumSize(300,400);

    listWidget = new QListWidget(this);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &SubcircuitLibarary::doubleClickedOnItem);

    if (circuit) {
        for (const auto& pair: circuit->subcircuitDefinitions) {
            QString subcircuitName = QString::fromStdString(pair.first);
            QListWidgetItem* subcircuitItem = new QListWidgetItem(subcircuitName);
            subcircuitItem->setData(Qt::UserRole, "U:" + subcircuitName);
            listWidget->addItem(subcircuitItem);
        }
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
}

void SubcircuitLibarary::doubleClickedOnItem(QListWidgetItem* item) {
    QString componentType = item->data(Qt::UserRole).toString();
    emit componentSelected(componentType);
    accept();
}