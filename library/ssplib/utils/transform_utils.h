#ifndef TRANSFORM_UTILS_H
#define TRANSFORM_UTILS_H

#include <QTransform>

namespace ssplib {

namespace utils {

QTransform parseTransformationMatrix(const QStringRef &value);

struct Transform
{
    QString value;
    QTransform matrix;
};

Transform combineTransform(const Transform& parent, const QString& val);

} // namespace utils

} // namespace ssplib

#endif // TRANSFORM_UTILS_H
