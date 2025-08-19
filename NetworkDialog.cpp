#include "NetworkDialog.h"
#include <qvalidator.h>

NetworkDialog::NetworkDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Network Configuration");

    serverRadioButton = new QRadioButton("Server (Host)");
    clientRadioButton = new QRadioButton("Client (Connect)");
    serverRadioButton->setChecked(true); // Default to Server mode

    ipAddressEdit = new QLineEdit(this);
    portEdit = new QLineEdit(this);

    ipAddressEdit->setPlaceholderText("e.g. 192.168.1.100");
    portEdit->setPlaceholderText("e.g. 12345");
    portEdit->setValidator(new QIntValidator(1024, 65535, this));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Mode:", serverRadioButton);
    formLayout->addRow("", clientRadioButton);
    formLayout->addRow("IP Address:", ipAddressEdit);
    formLayout->addRow("Port:", portEdit);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NetworkDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NetworkDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    // Disable IP for server mode as it uses the local machine's IP
    connect(serverRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if (checked) {
            ipAddressEdit->setEnabled(false);
            ipAddressEdit->clear();
        } else {
            ipAddressEdit->setEnabled(true);
        }
    });
}

bool NetworkDialog::isServerMode() const {
    return serverRadioButton->isChecked();
}

QString NetworkDialog::getIpAddress() const {
    return ipAddressEdit->text();
}

int NetworkDialog::getPort() const {
    return portEdit->text().toInt();
}