#ifndef SSPLIB_ITEMTYPES_H
#define SSPLIB_ITEMTYPES_H

#include <QString>
#include <QPainterPath>
#include <QRgb>

#ifdef SSPLIB_ENABLE_EDITING
#include <QDomElement>
#endif

namespace ssplib {

//Common Types

typedef qint64 db_id;

constexpr QRgb whiteRGB = qRgb(255, 255, 255);

enum class Side : qint8
{
    West = 0,
    East = 1,
    NSides
};

struct ElementPath
{
#ifdef SSPLIB_ENABLE_EDITING
    QDomElement elem;
#endif
    QPainterPath path;
    double strokeWidth = 0;
};

struct ItemBase
{
    QVector<ElementPath> elements;
    db_id itemId = 0;
    bool visible = false;
};

struct LabelItem : ItemBase
{
    QChar gateLetter;
    QString labelText;
};

struct TrackBaseItem : ItemBase
{
    db_id jobId = 0;
    QRgb color = whiteRGB;
    QString jobName;
};

struct TrackItem : TrackBaseItem
{
    QString trackName;
    int trackPos = 0;
};

struct LineTrackItem : TrackBaseItem
{
    db_id gateId = 0;
    QChar gateLetter;
    int gateTrackPos = 0;
};

struct TrackConnectionInfo
{
    db_id trackId = 0;
    db_id gateId = 0;
    int stationTrackPos = 0;
    int gateTrackPos = 0;
    QChar gateLetter;
    Side trackSide = Side::NSides;
};

struct TrackConnectionItem : TrackBaseItem
{
    TrackConnectionInfo info;
};

//Operators

//ElementPath
inline bool operator==(const ElementPath& left, const ElementPath& right)
{
    return left.path == right.path;
}

//LabelItem
inline bool operator<(const LabelItem& left, const LabelItem& right)
{
    return left.gateLetter < right.gateLetter;
}

//TrackItem
inline bool operator<(const TrackItem& left, const TrackItem& right)
{
    return left.trackPos < right.trackPos;
}

//LineTrackItem
inline bool operator<(const LineTrackItem& left, const LineTrackItem& right)
{
    if(left.gateLetter == right.gateLetter)
        return left.gateTrackPos < right.gateTrackPos;
    return left.gateLetter < right.gateLetter;
}

//TrackConnectionInfo
inline bool operator==(const TrackConnectionInfo& left, const TrackConnectionInfo& right)
{
    return left.stationTrackPos == right.stationTrackPos &&
           left.gateTrackPos == right.gateTrackPos &&
           left.gateLetter == right.gateLetter;
}

inline bool operator!=(const TrackConnectionInfo& left, const TrackConnectionInfo& right)
{
    return left.stationTrackPos != right.stationTrackPos ||
           left.gateTrackPos != right.gateTrackPos ||
           left.gateLetter != right.gateLetter;
}

inline bool operator<(const TrackConnectionInfo& left, const TrackConnectionInfo& right)
{
    if(left.stationTrackPos == right.stationTrackPos)
    {
        if(left.gateLetter == right.gateLetter)
            return left.gateTrackPos < right.gateTrackPos;

        return left.gateLetter < right.gateLetter;
    }

    return left.stationTrackPos < right.stationTrackPos;
}

//TrackConnectionItem
inline bool operator<(const TrackConnectionItem& left, const TrackConnectionItem& right)
{
    return left.info < right.info;
}

} // namespace ssplib

#endif // SSPLIB_ITEMTYPES_H
