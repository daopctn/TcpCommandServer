#include "streamserver.h"
#include "protocol.h"

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>

// ─── Construction ─────────────────────────────────────────────────

StreamServer::StreamServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_client(nullptr),
      m_timer(new QTimer(this))
{
    m_config.load("config.json");
    loadCsvFiles();
    initCsv();

    m_timer->setInterval(m_config.interval_ms);
    connect(m_timer, &QTimer::timeout, this, &StreamServer::sendRandomMessage);
    connect(m_server, &QTcpServer::newConnection, this, &StreamServer::onNewConnection);

    if (m_server->listen(QHostAddress::LocalHost, 8889)) {
        qDebug() << "[SERVER] Listening on localhost:8889 — waiting for client...";
    } else {
        qCritical() << "[SERVER] Failed to bind port 8889:" << m_server->errorString();
    }
}

// ─── CSV Loading ──────────────────────────────────────────────────

void StreamServer::loadCsvFiles()
{
    // Track
    {
        QFile f("data/track.csv");
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s.readLine(); // skip header
            while (!s.atEnd()) {
                QStringList p = s.readLine().split(',');
                if (p.size() < 6) continue;
                TrackRow r;
                r.f1  = static_cast<quint16>(p[0].toUShort());
                r.f2  = static_cast<quint8> (p[1].toUShort());
                r.f3  = static_cast<quint16>(p[2].toUShort());
                r.f4a = static_cast<quint8> (p[3].toUShort());
                r.f4b = static_cast<quint8> (p[4].toUShort());
                r.f4c = static_cast<quint8> (p[5].toUShort());
                m_trackRows.append(r);
            }
            qDebug() << "[CSV] Loaded" << m_trackRows.size() << "track rows";
        } else {
            qWarning() << "[CSV] Cannot open data/track.csv";
        }
    }

    // Track2
    {
        QFile f("data/track2.csv");
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s.readLine();
            while (!s.atEnd()) {
                QStringList p = s.readLine().split(',');
                if (p.size() < 6) continue;
                TrackRow r;
                r.f1  = static_cast<quint16>(p[0].toUShort());
                r.f2  = static_cast<quint8> (p[1].toUShort());
                r.f3  = static_cast<quint16>(p[2].toUShort());
                r.f4a = static_cast<quint8> (p[3].toUShort());
                r.f4b = static_cast<quint8> (p[4].toUShort());
                r.f4c = static_cast<quint8> (p[5].toUShort());
                m_track2Rows.append(r);
            }
            qDebug() << "[CSV] Loaded" << m_track2Rows.size() << "track2 rows";
        } else {
            qWarning() << "[CSV] Cannot open data/track2.csv";
        }
    }

    // Angle
    {
        QFile f("data/angle.csv");
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s.readLine();
            while (!s.atEnd()) {
                QString line = s.readLine().trimmed();
                if (line.isEmpty()) continue;
                m_angleRows.append(static_cast<quint16>(line.toUShort()));
            }
            qDebug() << "[CSV] Loaded" << m_angleRows.size() << "angle rows";
        } else {
            qWarning() << "[CSV] Cannot open data/angle.csv";
        }
    }

    // Clutter
    {
        QFile f("data/clutter.csv");
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s.readLine();
            while (!s.atEnd()) {
                QStringList p = s.readLine().split(',');
                if (p.size() < 5) continue;
                ClutterRow r;
                r.value1 = static_cast<quint16>(p[0].toUShort());
                r.v2a    = static_cast<quint8> (p[1].toUShort());
                r.v2b    = static_cast<quint8> (p[2].toUShort());
                r.v2c    = static_cast<quint8> (p[3].toUShort());
                r.value3 = static_cast<quint16>(p[4].toUShort());
                m_clutterRows.append(r);
            }
            qDebug() << "[CSV] Loaded" << m_clutterRows.size() << "clutter rows";
        } else {
            qWarning() << "[CSV] Cannot open data/clutter.csv";
        }
    }

    // Video2  — format: header,entry1;entry2;...
    {
        QFile f("data/video2.csv");
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s.readLine();
            while (!s.atEnd()) {
                QString line = s.readLine().trimmed();
                if (line.isEmpty()) continue;
                int comma = line.indexOf(',');
                if (comma < 0) continue;
                Video2Row r;
                r.header = static_cast<quint16>(line.left(comma).toUShort());
                QString entriesPart = line.mid(comma + 1);
                entriesPart.remove('"');
                for (const QString &token : entriesPart.split(';', QString::SkipEmptyParts)) {
                    quint32 v = token.trimmed().toUInt();
                    Protocol::Video2Entry e;
                    e.a = static_cast<quint8>((v >> 16) & 0xFF);
                    e.b = static_cast<quint8>((v >>  8) & 0xFF);
                    e.c = static_cast<quint8>( v        & 0xFF);
                    r.entries.append(e);
                }
                m_video2Rows.append(r);
            }
            qDebug() << "[CSV] Loaded" << m_video2Rows.size() << "video2 rows";
        } else {
            qWarning() << "[CSV] Cannot open data/video2.csv";
        }
    }
}

// ─── CSV Log ──────────────────────────────────────────────────────

void StreamServer::initCsv()
{
    m_csvFile.setFileName("stream_log.csv");
    bool exists = m_csvFile.exists();
    m_csvFile.open(QIODevice::Append | QIODevice::Text);
    m_csvStream.setDevice(&m_csvFile);

    if (!exists) {
        m_csvStream << "timestamp,type,"
                       "f1,f2,f3,f4a,f4b,f4c,"
                       "angle,"
                       "value1,v2a,v2b,v2c,value3,"
                       "video2_header,video2_entries\n";
        m_csvStream.flush();
    }
}

// ─── Connection handling ───────────────────────────────────────────

void StreamServer::onNewConnection()
{
    m_client = m_server->nextPendingConnection();
    qDebug() << "[SERVER] Client connected from"
             << m_client->peerAddress().toString() << ":" << m_client->peerPort();

    connect(m_client, &QTcpSocket::readyRead,    this, &StreamServer::onClientData);
    connect(m_client, &QTcpSocket::disconnected, this, &StreamServer::onClientDisconnected);
    connect(m_client,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &StreamServer::onClientError);

    m_server->pauseAccepting();
    sendRandomMessage();
    m_timer->start();
}

// ─── Send ──────────────────────────────────────────────────────────

void StreamServer::sendRandomMessage()
{
    if (!m_client || m_client->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray frame;
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString csvRow;

    switch (0 /* m_typeIdx % 5 */) {

    case 0: {   // Track
        if (m_trackRows.isEmpty()) break;
        const TrackRow &r = m_trackRows[m_trackIdx % m_trackRows.size()];
        frame = Protocol::buildTrack(r.f1, r.f2, r.f3, r.f4a, r.f4b, r.f4c);
        qDebug() << "[SERVER] Sent Track:"
                 << "f1=" << r.f1 << "f2=" << r.f2 << "f3=" << r.f3
                 << "f4a=" << r.f4a << "f4b=" << r.f4b << "f4c=" << r.f4c;
        csvRow = QString("%1,track,%2,%3,%4,%5,%6,%7,,,,,,,,")
                     .arg(timestamp).arg(r.f1).arg(r.f2).arg(r.f3)
                     .arg(r.f4a).arg(r.f4b).arg(r.f4c);
        ++m_trackIdx;
        break;
    }

    case 1: {   // Track2
        if (m_track2Rows.isEmpty()) break;
        const TrackRow &r = m_track2Rows[m_track2Idx % m_track2Rows.size()];
        frame = Protocol::buildTrack2(r.f1, r.f2, r.f3, r.f4a, r.f4b, r.f4c);
        qDebug() << "[SERVER] Sent Track2:"
                 << "f1=" << r.f1 << "f2=" << r.f2 << "f3=" << r.f3
                 << "f4a=" << r.f4a << "f4b=" << r.f4b << "f4c=" << r.f4c;
        csvRow = QString("%1,track2,%2,%3,%4,%5,%6,%7,,,,,,,,")
                     .arg(timestamp).arg(r.f1).arg(r.f2).arg(r.f3)
                     .arg(r.f4a).arg(r.f4b).arg(r.f4c);
        ++m_track2Idx;
        break;
    }

    case 2: {   // Angle
        if (m_angleRows.isEmpty()) break;
        quint16 angle = m_angleRows[m_angleIdx % m_angleRows.size()];
        frame = Protocol::buildAngle(angle);
        qDebug() << "[SERVER] Sent Angle: angle=" << angle;
        csvRow = QString("%1,angle,,,,,,,%2,,,,,,,").arg(timestamp).arg(angle);
        ++m_angleIdx;
        break;
    }

    case 3: {   // Clutter
        if (m_clutterRows.isEmpty()) break;
        const ClutterRow &r = m_clutterRows[m_clutterIdx % m_clutterRows.size()];
        frame = Protocol::buildClutter(r.value1, r.v2a, r.v2b, r.v2c, r.value3);
        qDebug() << "[SERVER] Sent Clutter:"
                 << "value1=" << r.value1
                 << "v2a=" << r.v2a << "v2b=" << r.v2b << "v2c=" << r.v2c
                 << "value3=" << r.value3;
        csvRow = QString("%1,clutter,,,,,,,,%2,%3,%4,%5,%6,,")
                     .arg(timestamp).arg(r.value1)
                     .arg(r.v2a).arg(r.v2b).arg(r.v2c).arg(r.value3);
        ++m_clutterIdx;
        break;
    }

    case 4: {   // Video2
        if (m_video2Rows.isEmpty()) break;
        const Video2Row &r = m_video2Rows[m_video2Idx % m_video2Rows.size()];
        frame = Protocol::buildVideo2(r.header, r.entries);
        QStringList entryList;
        for (const auto &e : r.entries) {
            quint32 v = (static_cast<quint32>(e.a) << 16)
                      | (static_cast<quint32>(e.b) <<  8)
                      |  static_cast<quint32>(e.c);
            entryList << QString::number(v);
        }
        qDebug() << "[SERVER] Sent Video2: header=" << r.header
                 << "entries=" << r.entries.size();
        csvRow = QString("%1,video2,,,,,,,,,,,,,,%2,\"%3\"")
                     .arg(timestamp).arg(r.header).arg(entryList.join(';'));
        ++m_video2Idx;
        break;
    }

    default: break;
    }

    ++m_typeIdx;

    if (frame.isEmpty())
        return;

    m_csvStream << csvRow << "\n";
    m_csvStream.flush();

    m_client->write(frame);
    m_client->flush();
}

// ─── Socket events ────────────────────────────────────────────────

void StreamServer::onClientData()
{
    QByteArray data = m_client->readAll();
    qDebug() << "[CLIENT REPLY]" << data.size() << "bytes:" << data.toHex(' ');
}

void StreamServer::onClientDisconnected()
{
    m_timer->stop();
    qDebug() << "[SERVER] Client disconnected.";
    m_client->deleteLater();
    m_client = nullptr;
    m_server->resumeAccepting();
    qDebug() << "[SERVER] Waiting for new connection...";
}

void StreamServer::onClientError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    qDebug() << "[SERVER] Socket error:" << m_client->errorString();
    m_timer->stop();
    m_client->abort();
}
