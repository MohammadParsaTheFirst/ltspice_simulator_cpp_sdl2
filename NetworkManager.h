#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QObject>
#include <QDataStream>
#include <QThread>
#include "Circuit.h"

enum class NetworkRole {
    None,
    Server,
    Client
};

enum class MessageType {
    VoltageSource,
    CircuitFile,
    SignalData,
    ConnectionRequest,
    ConnectionAccepted,
    ConnectionRejected
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(Circuit* circuit, QObject* parent = nullptr);
    ~NetworkManager();

    bool startServer(quint16 port);
    bool connectToServer(const QString& host, quint16 port);
    void disconnect();
    void sendVoltageSource(const QString& name, const QString& node1, const QString& node2,
                          double value, bool isSinusoidal = false,
                          double offset = 0.0, double amplitude = 0.0, double frequency = 0.0);
    void sendCircuitFile();
    void sendSignalData(const std::map<double, double>& signalData, const QString& signalName);

    NetworkRole getRole() const { return role; }
    bool isConnected() const { return connected; }

signals:
    void connectionStatusChanged(bool connected, const QString& message);
    void voltageSourceReceived(const QString& name, const QString& node1, const QString& node2,
                              double value, bool isSinusoidal, double offset, double amplitude, double frequency);
    void circuitFileReceived();
    void signalDataReceived(const std::map<double, double>& data, const QString& signalName);

private slots:
    void newConnection();
    void readyRead();
    void socketError(QAbstractSocket::SocketError error);
    void socketDisconnected();

private:
    void processMessage(const QByteArray& message);
    void sendMessage(MessageType type, const QByteArray& data = QByteArray());

    QTcpServer* server;
    QTcpSocket* clientSocket;
    Circuit* circuit;
    NetworkRole role;
    bool connected;
};

#endif // NETWORKMANAGER_H