#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>

class TcpClient : public QObject {
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    void connectToServer(const QString& ipAddress, int port);

    signals:
        void voltageReceived(double voltage);
    void logMessage(const QString& msg);

    private slots:
        void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
};

#endif // TCPCLIENT_H