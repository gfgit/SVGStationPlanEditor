#ifndef SVGUTILS_H
#define SVGUTILS_H

#include <QDomElement>
#include <QPainterPath>

namespace utils
{

bool parseNumberAndAdvance(double &outVal, QStringRef &str);

bool parsePointAndAdvance(QPointF &outPoint, QStringRef &str);

bool convertElementToPath(const QDomElement& e, QPainterPath& path);

} // namespace utils

#endif // SVGUTILS_H
