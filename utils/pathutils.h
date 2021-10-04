#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <QPainterPath>

bool cutPathAtPoint(const QPointF& p, const double threshold, const QPainterPath& src, QPainterPath& dest, QPainterPath& rest);

#endif // PATHUTILS_H
