#ifndef SVGUTILS_H
#define SVGUTILS_H

#include <QDomElement>
#include <QPainterPath>

#include "nodefindertypes.h"

namespace utils
{

bool parseNumberAndAdvance(double &outVal, QStringRef &str);

bool parsePointAndAdvance(QPointF &outPoint, QStringRef &str);

bool convertElementToPath(const QDomElement& e, QPainterPath& path);

bool parseTrackConnectionAttribute(const QString& value, QVector<TrackConnectionInfo>& outVec);

QString trackConnInfoToString(const QVector<TrackConnectionInfo>& vec);

bool parseStrokeWidth(const ElementPath &e, double& outVal);

bool convertPathToSVG(const QPainterPath& path, QString& outD);

} // namespace utils

#endif // SVGUTILS_H
