#include "NetworkDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

NetworkDialog::NetworkDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Network Configuration");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Server/Client selection
    QHBoxLayout* roleLayout = new QHBoxLayout();
    serverRadio = new QRadioButton("Server", this);
    clientRadio = new QRadioButton("Client", this);
    clientRadio->setChecked(true);
    roleLayout->addWidget(serverRadio);
    roleLayout->addWidget(clientRadio);
    mainLayout->addLayout(roleLayout);

    // Host input
    QFormLayout* formLayout = new QFormLayout();
    hostLabel = new QLabel("Server Host:", this);
    hostEdit = new QLineEdit("localhost", this);
    formLayout->addRow(hostLabel, hostEdit);
    mainLayout->addLayout(formLayout);

    // Port input
    portEdit = new QLineEdit("12345", this);
    portEdit->setValidator(new QIntValidator(1024, 65535, this));
    formLayout->addRow("Port:", portEdit);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    connectButton = new QPushButton("Connect", this);
    cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(serverRadio, &QRadioButton::toggled, this, &NetworkDialog::toggleServerClient);
    connect(connectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    toggleServerClient();
}

NetworkDialog::~NetworkDialog() {
}

QString NetworkDialog::getHost() const {
    return hostEdit->text();
}

quint16 NetworkDialog::getPort() const {
    return portEdit->text().toUShort();
}

bool NetworkDialog::isServer() const {
    return serverRadio->isChecked();
}

void NetworkDialog::toggleServerClient() {
    bool isClient = clientRadio->isChecked();
    hostLabel->setEnabled(isClient);
    hostEdit->setEnabled(isClient);
}