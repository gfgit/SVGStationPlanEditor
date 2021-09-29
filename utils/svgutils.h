#ifndef SVGUTILS_H
#define SVGUTILS_H

#include <QDomElement>
#include <QPainterPath>

#include "nodefinderutils.h"

namespace utils
{

bool parseNumberAndAdvance(double &outVal, QStringRef &str);

bool parsePointAndAdvance(QPointF &outPoint, QStringRef &str);

bool convertElementToPath(const QDomElement& e, QPainterPath& path);

bool parseTrackConnectionAttribute(const QString& value, QVector<TrackConnectionInfo>& outVec);

bool parseStrokeWidth(const ElementPath &e, double& outVal);

QString trackConnInfoToString(const QVector<TrackConnectionInfo>& vec);

} // namespace utils

#endif // SVGUTILS_H
