#include "TransientDialog.h"

TransientDialog::TransientDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Transient Analysis Settings");

    startTimeEdit = new QLineEdit(this);
    stopTimeEdit = new QLineEdit(this);
    stepTimeEdit = new QLineEdit(this);
    parameterEdit = new QLineEdit(this);

    // Set default values for convenience
    startTimeEdit->setText("0.0");
    stopTimeEdit->setText("1.0");
    stepTimeEdit->setText("0.001");
    parameterEdit->setPlaceholderText("e.g. V(n2) or I(R1)");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Start Time:", startTimeEdit);
    formLayout->addRow("Stop Time:", stopTimeEdit);
    formLayout->addRow("Step Time:", stepTimeEdit);
    formLayout->addRow("Parameter to Plot:", parameterEdit);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TransientDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TransientDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

TransientDialog::~TransientDialog() {}

double TransientDialog::getStartTime() const {
    return startTimeEdit->text().toDouble();
}

double TransientDialog::getStopTime() const {
    return stopTimeEdit->text().toDouble();
}

double TransientDialog::getStepTime() const {
    return stepTimeEdit->text().toDouble();
}

QString TransientDialog::getParameter() const {
    return parameterEdit->text();
}
