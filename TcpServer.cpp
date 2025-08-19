#include "TcpServer.h"

#include <iostream>
#include <QString>
#include <qiodevice.h>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent), clientSocket(nullptr) {}


void TcpServer::startServer(int port) {
    if (this->listen(QHostAddress::Any, port)) {
        //emit logMessage(QStringLiteral("Server started on port %1. Waiting for a client...").arg(port));
        emit logMessage(QString::asprintf("Server started on port %d. Waiting for a client...", port));

        connect(this, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    } else {
        emit logMessage(QStringLiteral("Server could not start: %1.").arg(this->errorString()));
    }
}

void TcpServer::onNewConnection() {
    if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
        // Disconnect previous client if a new one connects
        clientSocket->disconnectFromHost();
    }
    clientSocket = this->nextPendingConnection();
    if (!clientSocket) {
        emit logMessage(QStringLiteral("Failed to get pending connection."));
        return;
    }
    connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServer::onSocketDisconnected);
    QString clientInfo = QStringLiteral("Client connected from %1:%2")
                             .arg(clientSocket->peerAddress().toString(), QString::number(clientSocket->peerPort()));
    emit logMessage(clientInfo);
}

void TcpServer::onSocketDisconnected() {
    emit logMessage("Client disconnected.");
    clientSocket->deleteLater();
    clientSocket = nullptr;
}

void TcpServer::sendVoltageValue(double voltage) {
    if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << voltage;
        clientSocket->write(data);
    }
}
// void TCPServer::handleClient() {
//     char buffer[1024] = {};
//     recv(clientSocket, buffer, sizeof(buffer), 0);
//     std::cout << " "Received: << buffer << "\n";
//     std::string reply = server!" from "Hello;
//     send(clientSocket, reply.c_str(), reply.size(), 0);
// }