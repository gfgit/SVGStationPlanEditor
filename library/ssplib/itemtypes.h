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
    QList<ElementPath> elements;
    db_id itemId = 0;
    bool visible = false;
};

struct LabelItem : ItemBase
{
    QChar gateLetter;
    QString labelText;
    int gateOutTrkCount = 0;
    Side gateSide = Side::NSides;
};

struct TrackBaseItem : ItemBase
{
    QRgb color = whiteRGB;
    QString tooltip;
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

    //NOTE: this matches Database Data
    inline bool matchIDs(const TrackConnectionInfo& other) const
    {
        return trackId == other.trackId && gateId == other.gateId
               && gateTrackPos == other.gateTrackPos && trackSide == other.trackSide;
    }

    //NOTE: this matches SVG Data
    inline bool matchNames(const TrackConnectionInfo& other) const
    {
        return stationTrackPos == other.stationTrackPos && gateLetter == other.gateLetter
               && gateTrackPos == other.gateTrackPos && trackSide == other.trackSide;
    }

    template <typename Container>
    static inline void removeAllNames(Container &vec, const TrackConnectionInfo& info)
    {
        auto it =std::remove_if(vec.begin(), vec.end(),
                                 [info](const TrackConnectionInfo& other) -> bool
                                 {
                                     return info.matchNames(other);
                                 });
        vec.erase(it, vec.end());
    }
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
inline bool operator<(const TrackConnectionInfo& left, const TrackConnectionInfo& right)
{
    if(left.stationTrackPos == right.stationTrackPos)
    {
        if(left.trackSide == right.trackSide)
        {
            if(left.gateLetter == right.gateLetter)
                return left.gateTrackPos < right.gateTrackPos;

            return left.gateLetter < right.gateLetter;
        }

        return left.trackSide < right.trackSide;
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
