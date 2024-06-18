#ifndef SSPLIB_SVG_PATH_UTILS_H
#define SSPLIB_SVG_PATH_UTILS_H

#include <ssplib/itemtypes.h>
#include "xmlelement.h"
#include <QPointF>

namespace ssplib {

namespace utils {

bool parseNumberAndAdvance(double &outVal, QStringView &str);

bool parsePointAndAdvance(QPointF &outPoint, QStringView &str);

bool convertElementToPath(const XmlElement& e, QPainterPath& path);

bool parseTrackConnectionAttribute(const QString& value, QList<TrackConnectionInfo>& outVec);

QString trackConnInfoToString(const QList<TrackConnectionInfo>& vec);

struct ElementStyle
{
    //Give precedence to style if both found
    double styleStrokeWidth = -1;
    double normalStrokeWidth = -1;
};

ElementStyle parseStrokeWidthStyle(const utils::XmlElement &e, const ElementStyle& parentStyle, const QRectF& bounds);

bool parseStrokeWidth(const XmlElement &e, const ElementStyle& parentStyle, const QRectF &bounds, double& outVal);

#ifdef SSPLIB_ENABLE_EDITING
bool parseStrokeWidthRecursve(QDomElement &e, const QRectF &bounds, double& outVal);

bool convertPathToSVG(const QPainterPath &path, QString &outD);
#endif // SSPLIB_ENABLE_EDITING

} // namespace utils

} // namespace ssplib

#endif // SSPLIB_SVG_PATH_UTILS_H
