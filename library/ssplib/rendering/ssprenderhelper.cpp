#include "ssprenderhelper.h"

#include <QPainter>

#include <ssplib/stationplan.h>

void setFontSize(QPainter *painter, const QFont& originalFont, const QRectF& originalRect, const QString& text)
{
    QFont font = originalFont;
    painter->setFont(originalFont);

    QTextOption opt(Qt::AlignCenter);
    opt.setWrapMode(QTextOption::WordWrap);

    QRectF rect = painter->boundingRect(originalRect, text, opt);
    if(!originalRect.contains(rect))
    {
        qreal factorX = originalRect.width() / rect.width();
        qreal factorY = originalRect.height() / rect.height();

        qreal factor = qMin(factorX, factorY);
        font.setPointSizeF(originalFont.pointSize() * factor);
        painter->setFont(font);
    }
}

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

    QPen trackPen(plan->platformRGB);
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

            for(const auto& elem : std::as_const(item.elements))
            {
                // Do not draw path for labels
                // Make sure rect does not have null size
                QRectF r = elem.path.boundingRect();
                double minSz = qMax(elem.strokeWidth, 0.1);
                r.setSize(QSize(qMax(minSz, r.width()), qMax(minSz, r.height())));

                QString text = item.labelText;
                if(text.isEmpty())
                    text = fmt.arg(item.gateLetter);

                setFontSize(painter, f, r, text);
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

            if(item.color == whiteRGB)
                trackPen.setColor(plan->platformRGB);
            else
                trackPen.setColor(item.color);

            for(const auto& elem : std::as_const(item.elements))
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

            if(item.color == whiteRGB)
                trackPen.setColor(plan->platformRGB);
            else
                trackPen.setColor(item.color);

            for(const auto& elem : std::as_const(item.elements))
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
