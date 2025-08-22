#include "NetworkManager.h"
#include <QHostAddress>
#include <QFile>
#include <QFileInfo>

NetworkManager::NetworkManager(Circuit* circuit, QObject* parent)
    : QObject(parent), circuit(circuit), role(NetworkRole::None), connected(false) {
    server = nullptr;
    clientSocket = nullptr;
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::startServer(quint16 port) {
    if (server) {
        delete server;
        server = nullptr;
    }

    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, port)) {
        emit connectionStatusChanged(false, "Server failed to start: " + server->errorString());
        return false;
    }

    connect(server, &QTcpServer::newConnection, this, &NetworkManager::newConnection);
    role = NetworkRole::Server;
    emit connectionStatusChanged(true, "Server started on port " + QString::number(port));
    return true;
}

bool NetworkManager::connectToServer(const QString& host, quint16 port) {
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QTcpSocket::connected, this, [this]() {
        connected = true;
        emit connectionStatusChanged(true, "Connected to server");
    });
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::readyRead);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &NetworkManager::socketError);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::socketDisconnected);

    clientSocket->connectToHost(host, port);
    if (!clientSocket->waitForConnected(5000)) {
        emit connectionStatusChanged(false, "Connection timeout");
        return false;
    }

    role = NetworkRole::Client;
    return true;
}

void NetworkManager::disconnect() {
    if (server) {
        server->close();
        delete server;
        server = nullptr;
    }
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }
    connected = false;
    role = NetworkRole::None;
    emit connectionStatusChanged(false, "Disconnected");
}

void NetworkManager::sendVoltageSource(const QString& name, const QString& node1, const QString& node2,
                                      double value, bool isSinusoidal,
                                      double offset, double amplitude, double frequency) {
    if (!connected) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << name << node1 << node2 << value << isSinusoidal;
    if (isSinusoidal) {
        stream << offset << amplitude << frequency;
    }

    sendMessage(MessageType::VoltageSource, data);
}

void NetworkManager::sendCircuitFile() {
    if (!connected) return;

    // Save circuit to temporary file
    QString tempFile = QCoreApplication::applicationDirPath() + "/temp_circuit.psp";
    circuit->saveToFile(tempFile);

    QFile file(tempFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray circuitData = file.readAll();
    file.close();
    QFile::remove(tempFile);

    sendMessage(MessageType::CircuitFile, circuitData);
}

void NetworkManager::sendSignalData(const std::map<double, double>& signalData, const QString& signalName) {
    if (!connected) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << signalName;
    stream << static_cast<quint32>(signalData.size());
    for (const auto& point : signalData) {
        stream << point.first << point.second;
    }

    sendMessage(MessageType::SignalData, data);
}

void NetworkManager::newConnection() {
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
    }

    clientSocket = server->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::readyRead);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &NetworkManager::socketError);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::socketDisconnected);

    connected = true;
    emit connectionStatusChanged(true, "Client connected");

    // Send connection accepted message
    sendMessage(MessageType::ConnectionAccepted);
}

void NetworkManager::readyRead() {
    QByteArray message = clientSocket->readAll();
    processMessage(message);
}

void NetworkManager::socketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit connectionStatusChanged(false, "Socket error: " + clientSocket->errorString());
    disconnect();
}

void NetworkManager::socketDisconnected() {
    emit connectionStatusChanged(false, "Disconnected from peer");
    disconnect();
}

void NetworkManager::processMessage(const QByteArray& message) {
    QDataStream stream(message);
    stream.setVersion(QDataStream::Qt_6_5);

    quint32 type;
    stream >> type;
    MessageType msgType = static_cast<MessageType>(type);

    switch (msgType) {
    case MessageType::VoltageSource: {
        QString name, node1, node2;
        double value;
        bool isSinusoidal;
        double offset = 0.0, amplitude = 0.0, frequency = 0.0;

        stream >> name >> node1 >> node2 >> value >> isSinusoidal;
        if (isSinusoidal) {
            stream >> offset >> amplitude >> frequency;
        }

        emit voltageSourceReceived(name, node1, node2, value, isSinusoidal, offset, amplitude, frequency);
        break;
    }
    case MessageType::CircuitFile: {
        QByteArray circuitData = message.mid(sizeof(quint32));
        QString tempFile = QCoreApplication::applicationDirPath() + "/received_circuit.psp";

        QFile file(tempFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(circuitData);
            file.close();
            circuit->loadFromFile(tempFile);
            QFile::remove(tempFile);
            emit circuitFileReceived();
        }
        break;
    }
    case MessageType::SignalData: {
        QString signalName;
        quint32 pointCount;
        std::map<double, double> signalData;

        stream >> signalName >> pointCount;
        for (quint32 i = 0; i < pointCount; ++i) {
            double x, y;
            stream >> x >> y;
            signalData[x] = y;
        }

        emit signalDataReceived(signalData, signalName);
        break;
    }
    case MessageType::ConnectionAccepted:
        emit connectionStatusChanged(true, "Connection accepted by server");
        break;
    case MessageType::ConnectionRejected:
        emit connectionStatusChanged(false, "Connection rejected by server");
        disconnect();
        break;
    default:
        break;
    }
}

void NetworkManager::sendMessage(MessageType type, const QByteArray& data) {
    if (!connected) return;

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << static_cast<quint32>(type);
    if (!data.isEmpty()) {
        stream.writeRawData(data.constData(), data.size());
    }

    clientSocket->write(message);
}