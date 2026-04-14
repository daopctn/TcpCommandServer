#ifndef COMMANDSERVER_H
#define COMMANDSERVER_H

#include <QObject>
#include "config.h"

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
    quint16     m_angle      = 0;
    quint16     m_videoValue = 0;
    Config      m_config;
};

#endif // COMMANDSERVER_H
