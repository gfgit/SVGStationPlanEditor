#ifndef SSPLIB_SVG_PATH_UTILS_H
#define SSPLIB_SVG_PATH_UTILS_H

#include "ssplib/itemtypes.h"
#include "xmlelement.h"
#include <QPointF>

namespace ssplib {

namespace utils {

bool parseNumberAndAdvance(double &outVal, QStringRef &str);

bool parsePointAndAdvance(QPointF &outPoint, QStringRef &str);

bool convertElementToPath(const XmlElement& e, QPainterPath& path);

bool parseTrackConnectionAttribute(const QString& value, QVector<TrackConnectionInfo>& outVec);

QString trackConnInfoToString(const QVector<TrackConnectionInfo>& vec);

bool parseStrokeWidth(const XmlElement &e, const QRectF &bounds, double& outVal);

#ifdef SSPLIB_ENABLE_EDITING
bool convertPathToSVG(const QPainterPath &path, QString &outD);
#endif // SSPLIB_ENABLE_EDITING

} // namespace utils

} // namespace ssplib

#endif // SSPLIB_SVG_PATH_UTILS_H
