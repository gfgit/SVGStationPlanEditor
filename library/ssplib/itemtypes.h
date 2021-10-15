#ifndef ITEMTYPES_H
#define ITEMTYPES_H

#include <QString>
#include <QPainterPath>

namespace ssplib {

//Common Types

typedef qint64 db_id;

struct ElementPath
{
    QPainterPath path;
    double strokeWidth = 0;
};

typedef struct ItemBase
{
    QVector<ElementPath> elements;
    db_id itemId = 0;
    bool visible = false;
} ItemBase;

typedef struct LabelItem : ItemBase
{
    QChar gateLetter;
    QString labelText;
} LabelItem;

typedef struct TrackItem : ItemBase
{
    QString trackName;
    int trackPos = 0;
} TrackItem;

typedef struct TrackConnectionInfo
{
    db_id trackId = 0;
    db_id gateId = 0;
    int stationTrackPos = 0;
    int gateTrackPos = 0;
    QChar gateLetter;
} TrackConnectionInfo;

typedef struct TrackConnectionItem : ItemBase
{
    TrackConnectionInfo info;
} TrackConnectionItem;

//Operators

inline bool operator==(const ElementPath& left, const ElementPath& right)
{
    return left.path == right.path;
}

inline bool operator<(const LabelItem& left, const LabelItem& right)
{
    return left.gateLetter < right.gateLetter;
}

inline bool operator<(const TrackItem& left, const TrackItem& right)
{
    return left.trackPos < right.trackPos;
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

inline bool operator<(const TrackConnectionItem& left, const TrackConnectionItem& right)
{
    return left.info < right.info;
}

inline bool operator==(const TrackConnectionInfo& left, const TrackConnectionInfo& right)
{
    return left.stationTrackPos == right.stationTrackPos &&
           left.gateTrackPos == right.gateTrackPos &&
           left.gateLetter == right.gateLetter;
}

} // namespace ssplib

#endif // ITEMTYPES_H
