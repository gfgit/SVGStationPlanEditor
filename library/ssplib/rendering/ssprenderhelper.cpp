#include "ssprenderhelper.h"

#include <QPainter>

#include "../stationplan.h"

QTransform ssplib::SSPRenderHelper::getTranform(const QRectF &target, const QRectF &source)
{
    const double scaleFactor = target.width() / source.width();

    QTransform transform;
    transform.scale(target.width() / source.width(),
                    target.height() / source.height());
    QRectF c2 = transform.mapRect(source);

    transform.reset();
    transform.translate(target.x() - c2.x(),
                        target.y() - c2.y());
    transform.scale(scaleFactor, scaleFactor);
    return transform;
}

void ssplib::SSPRenderHelper::drawPlan(QPainter *painter, StationPlan *plan, const QRectF& target, const QRectF& source)
{
    static constexpr double PenWidthFactor = 1.5;

    painter->setTransform(getTranform(target, source));

    QPen trackPen(plan->platforRGB);
    trackPen.setCapStyle(Qt::RoundCap);

    painter->setPen(plan->labelRGB);

    //Draw labels
    if(plan->drawLabels)
    {
        QFont f;
        const QString fmt = QLatin1String("Label %1");

        const int count = plan->labels.count();
        for(int i = 0; i < count; i++)
        {
            const LabelItem &item = plan->labels.at(i);
            if(!item.visible || item.elements.isEmpty())
                continue; //Skip it

            for(const auto& elem : qAsConst(item.elements))
            {
                //Do not draw path for labels
                const QRectF r = elem.path.boundingRect();

                QString text = item.labelText;
                if(text.isEmpty())
                    text = fmt.arg(item.gateLetter);

                int sizeH = r.height() * 0.85;
                int sizeW = r.width() * 0.3;
                const int minPixelSize = 10;

                f.setPixelSize(qMax(minPixelSize, qMin(sizeH, sizeW)));
                painter->setFont(f);
                painter->drawText(r, text, QTextOption(Qt::AlignCenter));
            }

        }
    }

    //Draw tracks
    if(plan->drawTracks)
    {
        for(int i = 0; i < plan->platforms.count(); i++)
        {
            const TrackItem &item = plan->platforms.at(i);
            if(!item.visible || item.elements.isEmpty())
                continue; //Skip it

            for(const auto& elem : qAsConst(item.elements))
            {
                if(elem.strokeWidth == 0)
                    trackPen.setWidth(plan->platformPenWidth);
                else
                    trackPen.setWidthF(elem.strokeWidth * PenWidthFactor);
                painter->setPen(trackPen);

                painter->drawPath(elem.path);
            }
        }

        for(int i = 0; i < plan->trackConnections.count(); i++)
        {
            const TrackConnectionItem &item = plan->trackConnections.at(i);
            if(!item.visible || item.elements.isEmpty())
                continue; //Skip it

            for(const auto& elem : qAsConst(item.elements))
            {
                if(elem.strokeWidth == 0)
                    trackPen.setWidth(plan->platformPenWidth);
                else
                    trackPen.setWidthF(elem.strokeWidth * PenWidthFactor);
                painter->setPen(trackPen);

                painter->drawPath(elem.path);
            }
        }
    }

    painter->resetTransform();
}
