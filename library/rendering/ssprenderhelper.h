#ifndef SSPRENDERHELPER_H
#define SSPRENDERHELPER_H

class QPainter;
class QRectF;
class QTransform;

namespace ssplib {

class StationPlan;

class SSPRenderHelper
{
public:

    static QTransform getTranform(const QRectF &target, const QRectF &source);

    static void drawPlan(QPainter *painter, StationPlan *plan,
                         const QRectF &target, const QRectF &source);
};

} // namespace ssplib

#endif // SSPRENDERHELPER_H
