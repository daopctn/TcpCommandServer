#include "streamserver.h"
#include "protocol.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <cstdlib>  // rand, srand
#include <ctime>    // time

StreamServer::StreamServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_client(nullptr)
{
    srand(static_cast<unsigned>(time(nullptr)));
    m_config.load("config.json");

    connect(m_server, &QTcpServer::newConnection, this, &StreamServer::onNewConnection);

    if (m_server->listen(QHostAddress::LocalHost, 9999)) {
        qDebug() << "[SERVER] Listening on localhost:9999 — waiting for client...";
    } else {
        qCritical() << "[SERVER] Failed to bind port 9999:" << m_server->errorString();
    }
}

// ─── Connection handling ───────────────────────────────────────

void StreamServer::onNewConnection()
{
    m_client = m_server->nextPendingConnection();
    qDebug() << "[SERVER] Client connected from"
             << m_client->peerAddress().toString()
             << ":" << m_client->peerPort();

    connect(m_client, &QTcpSocket::readyRead,    this, &StreamServer::onClientData);
    connect(m_client, &QTcpSocket::disconnected, this, &StreamServer::onClientDisconnected);

    m_server->pauseAccepting();  // block new connections while client is connected
    sendRandomMessage();        // send once on connect
}

void StreamServer::sendRandomMessage()
{
    QByteArray frame;
    int type = rand() % 5;

    auto randInRange = [](qint64 min, qint64 max) -> qint64 {
        return min + rand() % (max - min + 1);
    };

    if (type == 0) {
        quint16 f1  = static_cast<quint16>(randInRange(m_config.track_f1.min,  m_config.track_f1.max));
        quint8  f2  = static_cast<quint8> (randInRange(m_config.track_f2.min,  m_config.track_f2.max));
        quint16 f3  = static_cast<quint16>(randInRange(m_config.track_f3.min,  m_config.track_f3.max));
        quint8  f4a = static_cast<quint8> (randInRange(m_config.track_f4a.min, m_config.track_f4a.max));
        quint8  f4b = static_cast<quint8> (randInRange(m_config.track_f4b.min, m_config.track_f4b.max));
        quint8  f4c = static_cast<quint8> (randInRange(m_config.track_f4c.min, m_config.track_f4c.max));
        frame = Protocol::buildTrack(f1, f2, f3, f4a, f4b, f4c);
        qDebug() << "[SERVER] Sent TrackMessage:"
                 << "f1=" << f1 << "f2=" << f2 << "f3=" << f3
                 << "f4a=" << f4a << "f4b=" << f4b << "f4c=" << f4c;
    } else if (type == 1) {
        quint16 f1  = static_cast<quint16>(randInRange(m_config.track2_f1.min,  m_config.track2_f1.max));
        quint8  f2  = static_cast<quint8> (randInRange(m_config.track2_f2.min,  m_config.track2_f2.max));
        quint16 f3  = static_cast<quint16>(randInRange(m_config.track2_f3.min,  m_config.track2_f3.max));
        quint8  f4a = static_cast<quint8> (randInRange(m_config.track2_f4a.min, m_config.track2_f4a.max));
        quint8  f4b = static_cast<quint8> (randInRange(m_config.track2_f4b.min, m_config.track2_f4b.max));
        quint8  f4c = static_cast<quint8> (randInRange(m_config.track2_f4c.min, m_config.track2_f4c.max));
        frame = Protocol::buildTrack2(f1, f2, f3, f4a, f4b, f4c);
        qDebug() << "[SERVER] Sent TrackMessage2:"
                 << "f1=" << f1 << "f2=" << f2 << "f3=" << f3
                 << "f4a=" << f4a << "f4b=" << f4b << "f4c=" << f4c;
    } else if (type == 2) {
        frame = Protocol::buildAngle(m_angle);
        qDebug() << "[SERVER] Sent AngleMessage:" << "angle=" << m_angle;
        m_angle = (m_angle >= m_config.angle.max) ? m_config.angle.min : m_angle + 1;
    } else if (type == 3) {
        quint16 value1 = static_cast<quint16>(randInRange(m_config.clutter_value1.min, m_config.clutter_value1.max));
        quint8  v2a    = static_cast<quint8> (randInRange(m_config.clutter_v2a.min,    m_config.clutter_v2a.max));
        quint8  v2b    = static_cast<quint8> (randInRange(m_config.clutter_v2b.min,    m_config.clutter_v2b.max));
        quint8  v2c    = static_cast<quint8> (randInRange(m_config.clutter_v2c.min,    m_config.clutter_v2c.max));
        quint16 value3 = static_cast<quint16>(randInRange(m_config.clutter_value3.min, m_config.clutter_value3.max));
        frame = Protocol::buildClutter(value1, v2a, v2b, v2c, value3);
        qDebug() << "[SERVER] Sent ClutterMessage:"
                 << "value1=" << value1
                 << "v2a=" << v2a << "v2b=" << v2b << "v2c=" << v2c
                 << "value3=" << value3;
    } else {
        int count = static_cast<int>(randInRange(m_config.video2_entry_count.min, m_config.video2_entry_count.max));
        QVector<Protocol::Video2Entry> entries;
        entries.reserve(count);
        for (int i = 0; i < count; ++i) {
            quint32 entryVal = static_cast<quint32>(randInRange(m_config.video2_entry.min, m_config.video2_entry.max));
            entries.append({ static_cast<quint8>((entryVal >> 16) & 0xFF),
                             static_cast<quint8>((entryVal >>  8) & 0xFF),
                             static_cast<quint8>( entryVal        & 0xFF) });
        }
        quint32 step = static_cast<quint32>(randInRange(m_config.video2_header_step.min, m_config.video2_header_step.max));
        quint32 next = m_videoValue + step;
        m_videoValue = (next > static_cast<quint32>(m_config.video2_header.max))
                       ? static_cast<quint16>(m_config.video2_header.min)
                       : static_cast<quint16>(next);
        frame = Protocol::buildVideo2(m_videoValue, entries);
        QDebug dbg = qDebug();
        dbg << "[SERVER] Sent VideoMessage2:"
            << "value1=" << m_videoValue << "entries=" << count << "\n";
        for (int i = 0; i < count; ++i) {
            quint32 val = (entries[i].a << 16) | (entries[i].b << 8) | entries[i].c;
            dbg << "  [" << i << "]=" << val << "\n";
        }
    }

    m_client->write(frame);
    m_client->flush();
}

void StreamServer::onClientData()
{
    QByteArray data = m_client->readAll();
    qDebug() << "[CLIENT REPLY]" << data.size() << "bytes:" << data.toHex(' ');
}

void StreamServer::onClientDisconnected()
{
    qDebug() << "[SERVER] Client disconnected.";
    m_client->deleteLater();
    m_client = nullptr;
    m_server->resumeAccepting();  // allow next connection
    qDebug() << "[SERVER] Waiting for new connection...";
}
