#ifndef NODEFINDERTYPES_H
#define NODEFINDERTYPES_H

#include <QString>
#include <QDomElement>
#include <QPainterPath>

struct ElementPath
{
    QDomElement elem;
    QPainterPath path;
    double strokeWidth = 0;
};

typedef struct ItemBase
{
    QVector<ElementPath> elements;
    bool visible;
} ItemBase;

typedef struct LabelItem : ItemBase
{
    QChar gateLetter;
} LabelItem;

typedef struct TrackItem : ItemBase
{
    QString trackName;
    int trackPos;
} TrackItem;

typedef struct TrackConnectionInfo
{
    int stationTrackPos;
    int gateTrackPos;
    QChar gateLetter;
} TrackConnectionInfo;

typedef struct TrackConnectionItem : ItemBase
{
    TrackConnectionInfo info;
} TrackConnectionItem;

//SVG attribute names
namespace svg_attr {

//Default
const QString ID = QLatin1String("id");
const QString XmlSpace = QLatin1String("xml:space");

//Custom
const QString LabelName = QLatin1String("labelname");
const QString TrackPos = QLatin1String("trackpos");
const QString TrackConnections = QLatin1String("trackconn");

//Text processing, attribute white list
const QStringList TSpanPassAttrs{"x", "y", "style", "fill", "stroke", "font-family", "font-size", "font-weight"};
const QStringList TSpanPassToTextAttrs{"x", "y"};

}

namespace svg_tag {

static const QString GroupTag = QLatin1String("g");
static const QString DefsTag = QLatin1String("defs");
static const QString FontTag = QLatin1String("font");
static const QString TextTag = QLatin1String("text");
static const QString TSpanTag = QLatin1String("tspan");
static const QString RectTag = QLatin1String("rect");
static const QString PathTag = QLatin1String("path");
static const QString LineTag = QLatin1String("line");
static const QString PolylineTag = QLatin1String("polyline");

}

//Operators

inline bool operator==(const ElementPath& left, const ElementPath& right)
{
    return left.elem == right.elem;
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

#endif // NODEFINDERTYPES_H
