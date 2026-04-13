#ifndef COMMANDSERVER_H
#define COMMANDSERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

class CommandServer : public QObject
{
    Q_OBJECT

public:
    explicit CommandServer(QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onClientData();
    void onClientDisconnected();

private:
    void sendRandomMessage();

    QTcpServer *m_server;
    QTcpSocket *m_client;
};

#endif // COMMANDSERVER_H
