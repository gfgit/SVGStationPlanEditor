#include "svgcreatorscene.h"

#include <QPainter>

#include <QGraphicsSceneMouseEvent>

#include <QDebug>

SvgCreatorScene::SvgCreatorScene(SvgCreatorManager *mgr, QObject *parent) :
    QGraphicsScene(parent),
    manager(mgr)
{

}

void SvgCreatorScene::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    isDrawingLine = true;
    lineStart = lineEnd = lineRoundedEnd = ev->scenePos();
    update();
}

void SvgCreatorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    if(isDrawingLine)
    {
        lineEnd = lineRoundedEnd = ev->scenePos();

        if(ev->modifiers().testFlag(Qt::ControlModifier))
        {
            //Round angle to 45 degrees
            QLineF line(lineStart, lineEnd);
            const double angle = line.angle();
            double roundedAngle = qRound(angle / 45.0) * 45;
            line.setAngle(roundedAngle);
            lineRoundedEnd = line.p2();
        }
        else
        {
            lineRoundedEnd = lineEnd;
        }

        update();
    }
}

void SvgCreatorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    isDrawingLine = false;
    lineStart = lineEnd = lineRoundedEnd = QPointF();
    update();
}

void SvgCreatorScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{

}

void SvgCreatorScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    //Draw page box of scene rect
    painter->setBrush(Qt::white);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(sceneRect());
}

void SvgCreatorScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if(isDrawingLine)
    {
        painter->setPen(QPen(Qt::red, 5));
        painter->drawLine(lineStart, lineEnd);

        painter->setPen(QPen(Qt::darkCyan, 5));
        painter->drawLine(lineStart, lineRoundedEnd);
    }
}
