#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

struct Range {
    qint64 min;
    qint64 max;
};

struct Config {
    // Track
    Range track_f1, track_f2, track_f3, track_f4a, track_f4b, track_f4c;

    // Track2
    Range track2_f1, track2_f2, track2_f3, track2_f4a, track2_f4b, track2_f4c;

    // Angle
    Range angle;

    // Clutter
    Range clutter_value1, clutter_v2a, clutter_v2b, clutter_v2c, clutter_value3;

    // Video2
    Range video2_header, video2_header_step, video2_entry, video2_entry_count;

    static Range readRange(const QJsonObject &obj, const QString &key)
    {
        QJsonObject r = obj[key].toObject();
        return { r["min"].toInt(), r["max"].toInt() };
    }

    bool load(const QString &path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "[CONFIG] Cannot open" << path;
            return false;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            qCritical() << "[CONFIG] Invalid JSON in" << path;
            return false;
        }

        QJsonObject root = doc.object();

        QJsonObject track = root["track"].toObject();
        track_f1  = readRange(track, "f1");
        track_f2  = readRange(track, "f2");
        track_f3  = readRange(track, "f3");
        track_f4a = readRange(track, "f4a");
        track_f4b = readRange(track, "f4b");
        track_f4c = readRange(track, "f4c");

        QJsonObject track2 = root["track2"].toObject();
        track2_f1  = readRange(track2, "f1");
        track2_f2  = readRange(track2, "f2");
        track2_f3  = readRange(track2, "f3");
        track2_f4a = readRange(track2, "f4a");
        track2_f4b = readRange(track2, "f4b");
        track2_f4c = readRange(track2, "f4c");

        QJsonObject ang = root["angle"].toObject();
        angle = { ang["min"].toInt(), ang["max"].toInt() };

        QJsonObject clutter = root["clutter"].toObject();
        clutter_value1 = readRange(clutter, "value1");
        clutter_v2a    = readRange(clutter, "v2a");
        clutter_v2b    = readRange(clutter, "v2b");
        clutter_v2c    = readRange(clutter, "v2c");
        clutter_value3 = readRange(clutter, "value3");

        QJsonObject video2 = root["video2"].toObject();
        video2_header      = readRange(video2, "header");
        video2_header_step = readRange(video2, "header_step");
        video2_entry       = readRange(video2, "entry");
        video2_entry_count = readRange(video2, "entry_count");

        qDebug() << "[CONFIG] Loaded from" << path;
        return true;
    }
};

#endif // CONFIG_H
