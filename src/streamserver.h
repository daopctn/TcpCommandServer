#ifndef STREAMSERVER_H
#define STREAMSERVER_H

#include <QObject>
#include <QTimer>
#include <QAbstractSocket>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include "config.h"
#include "protocol.h"

class QTcpServer;
class QTcpSocket;

class StreamServer : public QObject
{
    Q_OBJECT

public:
    explicit StreamServer(QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onClientData();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    void sendRandomMessage();
    void loadCsvFiles();
    void initCsv();

    // ─── CSV row types ─────────────────────────────────────────────
    struct TrackRow   { quint16 f1; quint8 f2; quint16 f3; quint8 f4a, f4b, f4c; };
    struct ClutterRow { quint16 value1; quint8 v2a, v2b, v2c; quint16 value3; };
    struct Video2Row  { quint16 header; QVector<Protocol::Video2Entry> entries; };

    QTcpServer *m_server;
    QTcpSocket *m_client;
    QTimer     *m_timer;
    Config      m_config;

    // ─── Data loaded from CSV files ────────────────────────────────
    QVector<TrackRow>   m_trackRows;
    QVector<TrackRow>   m_track2Rows;
    QVector<quint16>    m_angleRows;
    QVector<ClutterRow> m_clutterRows;
    QVector<Video2Row>  m_video2Rows;

    // ─── Playback indices (wrap around on overflow) ────────────────
    int m_typeIdx    = 0;
    int m_trackIdx   = 0;
    int m_track2Idx  = 0;
    int m_angleIdx   = 0;
    int m_clutterIdx = 0;
    int m_video2Idx  = 0;

    QFile       m_csvFile;
    QTextStream m_csvStream;
};

#endif // STREAMSERVER_H
