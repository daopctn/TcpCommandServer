#ifndef STREAMSERVER_H
#define STREAMSERVER_H

#include <QObject>
#include "config.h"

class QTcpServer;
class QTcpSocket;
class QTimer;

class StreamServer : public QObject
{
    Q_OBJECT

public:
    explicit StreamServer(QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onClientData();
    void onClientDisconnected();

private:
    void sendRandomMessage();

    QTcpServer *m_server;
    QTcpSocket *m_client;
    QTimer     *m_streamTimer;
    quint16     m_angle      = 0;
    quint16     m_videoValue = 0;
    Config      m_config;
};

#endif // STREAMSERVER_H
