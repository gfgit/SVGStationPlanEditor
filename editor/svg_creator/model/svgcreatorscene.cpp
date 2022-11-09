#include "svgcreatorscene.h"
#include "svg_creator/svgcreatormanager.h"

#include <QMenu>

#include <QPainter>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>

#include <QDebug>

SvgCreatorScene::SvgCreatorScene(SvgCreatorManager *mgr, QObject *parent) :
    QGraphicsScene(parent),
    manager(mgr)
{

}

void SvgCreatorScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *ev)
{
    QGraphicsItem *item = itemAt(ev->scenePos(), QTransform());
    if(!item)
        return;

    int idx = -1;
    for(int i = 0; i < manager->trackConnections.size(); i++)
    {
        if(manager->trackConnections.at(i).lineItem == item)
        {
            idx = i;
            break;
        }
    }

    if(idx == -1)
        return;

    ev->accept();

    QMenu menu(ev->widget());
    QAction *deleteItem = menu.addAction(tr("Delete Item"));

    QAction *chosenAction = menu.exec(ev->screenPos());
    if(chosenAction == deleteItem)
    {
        manager->trackConnections.removeAt(idx);
        removeItem(item);
        delete item;
        update();
    }
}

void SvgCreatorScene::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(isDrawingLine)
    {
        isDrawingLine = false;

        if(ev->button() == Qt::LeftButton)
        {
            addTrackConnection(QLineF(lineStartSnapped, lineRoundedEnd));
        }

        lineStart = lineEnd = lineStartSnapped = lineRoundedEnd = QPointF();
        update();
    }
    else if(ev->button() == Qt::LeftButton)
    {
        isDrawingLine = true;
        lineStart = lineEnd = lineRoundedEnd = ev->scenePos();

        lineStartSnapped = lineStart;
        if(!ev->modifiers().testFlag(Qt::ShiftModifier))
            snapToPoint(lineStartSnapped, lineStart);

        update();
    }
}

void SvgCreatorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    if(isDrawingLine)
    {
        lineEnd = ev->scenePos();

        //Use exact mouse pos
        lineRoundedEnd = lineEnd;

        if(ev->modifiers().testFlag(Qt::ControlModifier))
        {
            //Round angle to 45 degrees
            QLineF line(lineStart, lineEnd);
            const double angle = line.angle();
            double roundedAngle = qRound(angle / 45.0) * 45;
            line.setAngle(roundedAngle);
            lineRoundedEnd = line.p2();
        }

        if(!ev->modifiers().testFlag(Qt::ShiftModifier))
        {
            //Snap end to nearest track
            snapToPoint(lineRoundedEnd, lineStartSnapped);
        }

        update();
    }
}

void SvgCreatorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    QGraphicsScene::mouseReleaseEvent(ev);
}

void SvgCreatorScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    QGraphicsScene::mouseDoubleClickEvent(ev);
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
        painter->drawLine(lineStartSnapped, lineRoundedEnd);
    }
}

void SvgCreatorScene::addTrackConnection(const QLineF &line)
{
    QPen pen(Qt::darkBlue, 5);
    pen.setCapStyle(Qt::RoundCap);

    TrackConnectionItem item;
    item.lineItem = addLine(line, pen);

    manager->addTrackConnection(item);
}

inline QLineF mapLineToScene(QGraphicsLineItem *item)
{
    return QLineF(item->mapToScene(item->line().p1()),
                  item->mapToScene(item->line().p2()));
}

bool SvgCreatorScene::snapToPoint(QPointF &pos, const QPointF& startPos)
{
    const double MAX_DISTANCE = 15;

    QPointF nearestCandidate;
    double nearestDistance = -1;

    //Helper to filter candidates and keep the best one
    auto consider = [&nearestCandidate, &nearestDistance, pos](const QPointF& candidate)
    {
        const double distance = (candidate - pos).manhattanLength();
        if(nearestDistance == -1 || distance < nearestDistance)
        {
            nearestCandidate = candidate;
            nearestDistance = distance;
        }
    };

    //Find every item near us
    QRectF toleranceRect(QPointF(), QSize(MAX_DISTANCE, MAX_DISTANCE) * 1.5);
    toleranceRect.moveCenter(pos);
    auto list = items(toleranceRect, Qt::IntersectsItemShape);

    //Refine search by keeping best candidate
    for(auto item : list)
    {
        //Try to snap to line edges
        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
        if(!line)
            continue;

        const QLineF otherLine = mapLineToScene(line);

        consider(otherLine.p1());
        consider(otherLine.p2());
    }

    if(nearestDistance == -1 || nearestDistance > MAX_DISTANCE)
    {
        //Give up snapping, try extending line to intersect
        nearestDistance = -1;
        QLineF currentLine(startPos, pos);
        if(currentLine.isNull())
            return false; //No line to extend

        for(auto item : list)
        {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
            if(!line)
                continue;

            //Calculate intersection in scene coordinates
            QLineF otherLine = mapLineToScene(line);

            QPointF intersection;
            QLineF::IntersectionType res = currentLine.intersects(otherLine, &intersection);
            if(res == QLineF::NoIntersection)
                continue;

            //Check if is bounded at least for other line (which we cannot extend)
            double minVal = otherLine.x1();
            double maxVal = otherLine.x2();
            if(minVal > maxVal)
                qSwap(minVal, maxVal);

            if(intersection.x() < minVal || intersection.x() > maxVal)
                continue;

            minVal = otherLine.y1();
            maxVal = otherLine.y2();
            if(minVal > maxVal)
                qSwap(minVal, maxVal);

            if(intersection.y() < minVal || intersection.y() > maxVal)
                continue;

            consider(intersection);
        }

        if(nearestDistance == -1 || nearestDistance > MAX_DISTANCE)
            return false; //If still too far away, give up
    }

    pos = nearestCandidate;
    return true;
}
