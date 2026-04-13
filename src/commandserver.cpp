#include "commandserver.h"
#include "protocol.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <cstdlib>  // rand, srand
#include <ctime>    // time

CommandServer::CommandServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_client(nullptr)
{
    srand(static_cast<unsigned>(time(nullptr)));

    connect(m_server, &QTcpServer::newConnection, this, &CommandServer::onNewConnection);

    if (m_server->listen(QHostAddress::LocalHost, 9999)) {
        qDebug() << "[SERVER] Listening on localhost:9999 — waiting for client...";
    } else {
        qCritical() << "[SERVER] Failed to bind port 9999:" << m_server->errorString();
    }
}

// ─── Connection handling ───────────────────────────────────────

void CommandServer::onNewConnection()
{
    m_client = m_server->nextPendingConnection();
    qDebug() << "[SERVER] Client connected from"
             << m_client->peerAddress().toString()
             << ":" << m_client->peerPort();

    connect(m_client, &QTcpSocket::readyRead,    this, &CommandServer::onClientData);
    connect(m_client, &QTcpSocket::disconnected, this, &CommandServer::onClientDisconnected);

    m_server->pauseAccepting();  // block new connections while client is connected
    sendRandomMessage();        // send once on connect
}

void CommandServer::sendRandomMessage()
{
    QByteArray frame;
    int type = rand() % 5;

    if (type == 0) {
        frame = Protocol::buildTrack(
            static_cast<quint16>(rand() % 0xFFFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint16>(rand() % 0xFFFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF)
        );
        qDebug() << "[SERVER] Sent TrackMessage:" << frame.toHex(' ');
    } else if (type == 1) {
        frame = Protocol::buildTrack2(
            static_cast<quint16>(rand() % 0xFFFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint16>(rand() % 0xFFFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF)
        );
        qDebug() << "[SERVER] Sent TrackMessage2:" << frame.toHex(' ');
    } else if (type == 2) {
        frame = Protocol::buildAngle(static_cast<quint16>(rand() % 0xFFFF));
        qDebug() << "[SERVER] Sent AngleMessage:" << frame.toHex(' ');
    } else if (type == 3) {
        frame = Protocol::buildClutter(
            static_cast<quint16>(rand() % 0xFFFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint8> (rand() % 0xFF),
            static_cast<quint16>(rand() % 0xFFFF)
        );
        qDebug() << "[SERVER] Sent ClutterMessage:" << frame.toHex(' ');
    } else {
        int count = 1 + rand() % 5;  // 1 to 5 value2 entries
        QVector<Protocol::Video2Entry> entries;
        entries.reserve(count);
        for (int i = 0; i < count; ++i)
            entries.append({ static_cast<quint8>(rand() % 0xFF),
                             static_cast<quint8>(rand() % 0xFF),
                             static_cast<quint8>(rand() % 0xFF) });
        frame = Protocol::buildVideo2(static_cast<quint16>(rand() % 0xFFFF), entries);
        qDebug() << "[SERVER] Sent VideoMessage2 (" << count << "entries):" << frame.toHex(' ');
    }

    m_client->write(frame);
    m_client->flush();
}

void CommandServer::onClientData()
{
    QByteArray data = m_client->readAll();
    qDebug() << "[CLIENT REPLY]" << data.size() << "bytes:" << data.toHex(' ');
}

void CommandServer::onClientDisconnected()
{
    qDebug() << "[SERVER] Client disconnected.";
    m_client->deleteLater();
    m_client = nullptr;
    m_server->resumeAccepting();  // allow next connection
    qDebug() << "[SERVER] Waiting for new connection...";
}
