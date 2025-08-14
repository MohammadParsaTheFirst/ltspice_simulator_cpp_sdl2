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

NodeLibraryDialog::NodeLibraryDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Node library");
    setMinimumSize(300,400);

    listWidget = new QListWidget(this);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &NodeLibraryDialog::doubleClickedOnItem);

    QListWidgetItem* resistorItem = new QListWidgetItem("Resistor");
    QListWidgetItem* capacitorItem = new QListWidgetItem("Capacitor");
    QListWidgetItem* inductorItem = new QListWidgetItem("Inductor");
    QListWidgetItem* diodeItem = new QListWidgetItem("Diode");
    QListWidgetItem* voltageSourceItem = new QListWidgetItem("Independent voltage source");
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
    listWidget->addItem(currentSourceItem);
    listWidget->addItem(vcvsItem);
    listWidget->addItem(vccsItem);
    listWidget->addItem(ccvsItem);
    listWidget->addItem(cccsItem);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
}

void NodeLibraryDialog::doubleClickedOnItem(QListWidgetItem* item) {
    QString componentType = item->data(Qt::UserRole).toString();
    emit componentSelected(componentType);
    accept();
}

LabelDialog::LabelDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Node label");
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