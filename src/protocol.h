#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QDataStream>
#include <QVector>

namespace Protocol {

// ─── Message Types ─────────────────────────────────────────────
enum MessageType : quint8 {
    MSG_STATUS   = 0x01,  // payload: [uint16 code][uint16 flags]                          → 4 bytes
    MSG_POSITION = 0x02,  // payload: [float32 x][float32 y][float32 z]                   → 12 bytes
    MSG_COMMAND  = 0x03,  // payload: [uint8 cmdId]                                        → 1 byte
    MSG_TRACK      = 0x11,  // payload: [uint16][uint8][uint16][3 bytes]                          → 8 bytes
    MSG_ANGLE      = 0x12,  // payload: [uint16 value]                                            → 2 bytes
    MSG_CLUTTER    = 0x22,  // payload: [uint16 value1][3 bytes value2][uint16 value3]            → 7 bytes
    MSG_TRACK2     = 0xA1,  // payload: [uint16][uint8][uint16][3 bytes]                          → 8 bytes
    MSG_VIDEO2     = 0xA0,  // payload: [uint16 value1][value2: 3 bytes] × N                      → 2 + 3N bytes
};

// ─── Frame format: [TYPE:1][LENGTH:2][DATA:N] ──────────────────
// LENGTH = total message length (TYPE + LENGTH + DATA = 3 + N).

inline QByteArray buildFrame(quint8 type, const QByteArray &payload)
{
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    stream << type;
    stream << static_cast<quint16>(3 + payload.size());  // total: 1 + 2 + N
    frame.append(payload);

    return frame;
}

// ─── Per-type builders ─────────────────────────────────────────

inline QByteArray buildStatus(quint16 code, quint16 flags)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << code << flags;                          // 2 + 2 = 4 bytes
    return buildFrame(MSG_STATUS, payload);
}

inline QByteArray buildPosition(float x, float y, float z)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << x << y << z;                            // 4 + 4 + 4 = 12 bytes
    return buildFrame(MSG_POSITION, payload);
}

inline QByteArray buildCommand(quint8 cmdId)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << cmdId;                                   // 1 byte
    return buildFrame(MSG_COMMAND, payload);
}

inline QByteArray buildTrack(quint16 f1, quint8 f2, quint16 f3, quint8 f4a, quint8 f4b, quint8 f4c)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << f1 << f2 << f3 << f4a << f4b << f4c;     // 2 + 1 + 2 + 3 = 8 bytes
    return buildFrame(MSG_TRACK, payload);
}

inline QByteArray buildAngle(quint16 value)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << value;                                     // 2 bytes
    return buildFrame(MSG_ANGLE, payload);
}

inline QByteArray buildClutter(quint16 value1, quint8 v2a, quint8 v2b, quint8 v2c, quint16 value3)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << value1 << v2a << v2b << v2c << value3;    // 2 + 3 + 2 = 7 bytes
    return buildFrame(MSG_CLUTTER, payload);
}

inline QByteArray buildTrack2(quint16 f1, quint8 f2, quint16 f3, quint8 f4a, quint8 f4b, quint8 f4c)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << f1 << f2 << f3 << f4a << f4b << f4c;     // 2 + 1 + 2 + 3 = 8 bytes
    return buildFrame(MSG_TRACK2, payload);
}

struct Video2Entry {
    quint8 a, b, c;  // 3 bytes per value2
};

inline QByteArray buildVideo2(quint16 value1, const QVector<Video2Entry> &value2s)
{
    QByteArray payload;
    QDataStream s(&payload, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);
    s << value1;                                    // 2 bytes
    for (const Video2Entry &e : value2s)
        s << e.a << e.b << e.c;                     // 3 bytes each
    return buildFrame(MSG_VIDEO2, payload);
}

// ─── Parser helper ─────────────────────────────────────────────
// Minimum header size: 1 (type) + 2 (length) = 3 bytes
static const int HEADER_SIZE = 3;

} // namespace Protocol

#endif // PROTOCOL_H
