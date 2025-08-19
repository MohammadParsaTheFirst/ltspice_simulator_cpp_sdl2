#include "TcpClient.h"
#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::onError);
}

void TcpClient::connectToServer(const QString& ipAddress, int port) {
    emit logMessage("Attempting to connect to server...");
    socket->connectToHost(QHostAddress(ipAddress), port);
}

void TcpClient::onConnected() {
    emit logMessage("Connected to server.");
}

void TcpClient::onDisconnected() {
    emit logMessage("Disconnected from server.");
}

void TcpClient::onReadyRead() {
    QByteArray data = socket->readAll();
    QDataStream stream(&data, QIODevice::ReadOnly);
    double voltage;
    stream >> voltage;
    emit voltageReceived(voltage);
}

void TcpClient::onError(QAbstractSocket::SocketError socketError) {
    emit logMessage(QString("Socket error: %1").arg(socket->errorString()));
}