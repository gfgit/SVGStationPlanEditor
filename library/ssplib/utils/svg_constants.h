#ifndef SVG_CONSTANTS_H
#define SVG_CONSTANTS_H

#include <QString>
#include <QStringList>

namespace ssplib {

namespace svg_tags {

static const QString GroupTag = QLatin1String("g");
static const QString DefsTag = QLatin1String("defs");
static const QString FontTag = QLatin1String("font");
static const QString TextTag = QLatin1String("text");
static const QString TSpanTag = QLatin1String("tspan");
static const QString RectTag = QLatin1String("rect");
static const QString PathTag = QLatin1String("path");
static const QString LineTag = QLatin1String("line");
static const QString PolylineTag = QLatin1String("polyline");

} // namespace svg_tags

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

} // namespace ssplib

#endif // SVG_CONSTANTS_H
