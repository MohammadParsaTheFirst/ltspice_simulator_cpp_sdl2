#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QString>
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QTcpServer {
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    void startServer(int port);
    void sendVoltageValue(double voltage);

    signals:
        void logMessage(const QString& msg);

    private slots:
        void onNewConnection();
    void onSocketDisconnected();

private:
    QTcpSocket *clientSocket;
};

#endif // TCPSERVER_H