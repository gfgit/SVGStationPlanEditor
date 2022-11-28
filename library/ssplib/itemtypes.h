#ifndef SSPLIB_ITEMTYPES_H
#define SSPLIB_ITEMTYPES_H

#include <QString>
#include <QPainterPath>
#include <QRgb>

#ifdef SSPLIB_ENABLE_EDITING
#include <QDomElement>
#endif

#include "utils/svg_trackconn_util.h"

namespace ssplib {

//Common Types

constexpr QRgb whiteRGB = qRgb(255, 255, 255);

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
