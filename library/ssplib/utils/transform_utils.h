#ifndef TRANSFORM_UTILS_H
#define TRANSFORM_UTILS_H

#include <QTransform>

namespace ssplib {

namespace utils {

QMatrix parseTransformationMatrix(const QStringRef &value);

struct Transform
{
    QString value;
    QMatrix matrix;
};

Transform combineTransform(const Transform& parent, const QString& val);

} // namespace utils

} // namespace ssplib

#endif // TRANSFORM_UTILS_H
