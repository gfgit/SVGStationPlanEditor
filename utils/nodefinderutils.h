#ifndef NODEFINDERUTILS_H
#define NODEFINDERUTILS_H

#include <QString>
#include <QDomElement>
#include <QPainterPath>

struct ElementPath
{
    QDomElement elem;
    QPainterPath path;
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

//SVG attribute names
namespace svg_attr {

const QString ID = QLatin1String("id");
const QString LabelName = QLatin1String("labelname");
const QString TrackPos = QLatin1String("trackpos");

//Text processing, attribute white list
const QStringList TSpanPassAttrs{"x", "y", "fill", "stroke", "font-family", "font-size", "font-weight"};
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

#endif // NODEFINDERUTILS_H
