#include "svgcreatorscene.h"
#include "svg_creator/svgcreatormanager.h"

#include <QMenu>

#include <QPainter>

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include <QGraphicsLineItem>

#include <QDebug>

SvgCreatorScene::SvgCreatorScene(SvgCreatorManager *mgr, QObject *parent) :
    QGraphicsScene(parent),
    manager(mgr)
{

}

void SvgCreatorScene::setOverlayLines(const QLineF &lineA, const QLineF &lineB)
{
    overlayLineA = lineA;
    overlayLineB = lineB;
    hasOverlayLines = true;
    update();
}

void SvgCreatorScene::clearOverlayLines()
{
    hasOverlayLines = false;
    update();
}

void SvgCreatorScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *ev)
{
    int idx = -1;
    QGraphicsItem *item = itemAt(ev->scenePos(), QTransform());
    QPen originalPen;

    if(item)
    {
        if(auto line = qgraphicsitem_cast<QGraphicsLineItem *>(item))
        {
            //Highlight item
            originalPen = line->pen();
            line->setPen(QPen(Qt::red, originalPen.widthF() * 1.2));
        }

        //Find track connection
        for(int i = 0; i < manager->trackConnections.size(); i++)
        {
            if(manager->trackConnections.at(i)->lineItem == item)
            {
                idx = i;
                break;
            }
        }
    }

    ev->accept();

    QMenu menu(ev->widget());
    QAction *deleteItemAction = nullptr;
    QAction *splitTrackAction = nullptr;
    if(idx != -1)
    {
        //There's an item selected
        deleteItemAction = menu.addAction(tr("Delete Item"));
        splitTrackAction = menu.addAction(tr("Split Track"));
        menu.addSeparator();
    }

    QAction *enableDrawLineAction = menu.addAction(tr("Draw Tracks"));
    enableDrawLineAction->setCheckable(true);
    enableDrawLineAction->setChecked(m_toolMode == ToolMode::DrawTracks);

    QAction *chosenAction = menu.exec(ev->screenPos());

    //Reset highlight
    if(auto line = qgraphicsitem_cast<QGraphicsLineItem *>(item))
    {
        line->setPen(originalPen);
    }

    //Apply mode
    m_toolMode = enableDrawLineAction->isChecked() ? ToolMode::DrawTracks : ToolMode::MoveItems;

    if(deleteItemAction && chosenAction == deleteItemAction)
    {
        manager->trackConnections.removeAt(idx);
        removeItem(item);
        delete item;
        update();
        return;
    }
    else if(splitTrackAction && chosenAction == splitTrackAction)
    {
        emit manager->splitTrackRequested(manager->trackConnections[idx]);
    }
}

void SvgCreatorScene::keyPressEvent(QKeyEvent *ev)
{
    if(isDrawingLine && ev->matches(QKeySequence::Cancel))
    {
        endCurrentLineDrawing(false);
        return;
    }

    QGraphicsScene::keyPressEvent(ev);
}

void SvgCreatorScene::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(m_toolMode == ToolMode::DrawTracks)
    {
        ev->accept();

        if(isDrawingLine)
        {
            endCurrentLineDrawing(ev->button() == Qt::LeftButton);
            return;
        }
        else if(ev->button() == Qt::LeftButton)
        {
            //Start drawing a new line
            isDrawingLine = true;
            lineStart = lineEnd = lineRoundedEnd = ev->scenePos();

            lineStartSnapped = lineStart;
            if(!ev->modifiers().testFlag(Qt::ShiftModifier))
                snapToPoint(lineStartSnapped, lineStart);

            update();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(ev);
}

void SvgCreatorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    if(m_toolMode == ToolMode::DrawTracks && isDrawingLine)
    {
        ev->accept();
        lineEnd = ev->scenePos();

        //Use exact mouse pos
        lineRoundedEnd = lineEnd;

        if(ev->modifiers().testFlag(Qt::ControlModifier))
        {
            //Round angle to 45 degrees
            QLineF line(lineStartSnapped, lineEnd);
            const double angle = line.angle();
            const double length = line.length();
            int roundedAngle = qRound(angle / 45.0) * 45;
            if(roundedAngle < 0)
                roundedAngle += 360;
            roundedAngle = roundedAngle % 360;

            //Avoid QLineF::setAngle() because it causes slight imprecision
            //Due to finite floating point precision and it's more complex
            lineRoundedEnd = lineStartSnapped;
            if(roundedAngle == 90 || roundedAngle == 270)
            {
                lineRoundedEnd.ry() += (roundedAngle == 90 ? -1 : 1) * length; //Y inverted
            }
            else if(roundedAngle == 0 || roundedAngle == 180)
            {
                lineRoundedEnd.rx() += (roundedAngle == 0 ? 1 : -1) * length;
            }
            else
            {
                constexpr double SQRT_2 = 1.4142135623730950488;
                const double h = length * SQRT_2 / 2;
                lineRoundedEnd.rx() += ((roundedAngle == 45 || roundedAngle == 315) ? 1 : -1) * h;
                lineRoundedEnd.ry() += ((roundedAngle == 45 || roundedAngle == 135) ? -1 : 1) * h; //Y inverted
            }
        }

        if(!ev->modifiers().testFlag(Qt::ShiftModifier))
        {
            //Snap end to nearest track
            snapToPoint(lineRoundedEnd, lineStartSnapped);
        }

        update();
        return;
    }

    QGraphicsScene::mouseMoveEvent(ev);
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

    if(hasOverlayLines)
    {
        painter->setPen(QPen(Qt::green, 7));
        painter->drawLine(overlayLineA);
        painter->drawLine(overlayLineB);
    }
}

inline QLineF mapLineToScene(QGraphicsLineItem *item)
{
    return QLineF(item->mapToScene(item->line().p1()),
                  item->mapToScene(item->line().p2()));
}

void SvgCreatorScene::addTrackConnection(const QLineF &line)
{
    QPen pen(Qt::darkBlue, 5);
    pen.setCapStyle(Qt::RoundCap);

    //Add last part
    TrackConnectionItem *track = new TrackConnectionItem;
    track->lineItem = addLine(line, pen);
    manager->addTrackConnection(track);

    emit manager->splitTrackRequested(track, true);
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

        for(auto item : list)
        {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
            if(!line)
                continue;

            //Calculate intersection in scene coordinates
            QLineF otherLine = mapLineToScene(line);

            QPointF intersection;
            QLineF::IntersectionType res = QLineF::NoIntersection;

            if(currentLine.isNull())
            {
                //Null line, set perpendicular direction, then calculate
                //Perpendicular line has inverted slope
                const double x2 = startPos.x() + otherLine.dy();
                const double y2 = startPos.y() - otherLine.dx();
                QLineF perpendicular = currentLine;
                perpendicular.setP2(QPointF(x2, y2));
                res = perpendicular.intersects(otherLine, &intersection);
            }
            else
            {
                res = currentLine.intersects(otherLine, &intersection);
            }

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

void SvgCreatorScene::endCurrentLineDrawing(bool store)
{
    if(!isDrawingLine)
        return;

    //End current floating line
    isDrawingLine = false;

    if(store)
    {
        //Apply line with left button, cancel with right click
        addTrackConnection(QLineF(lineStartSnapped, lineRoundedEnd));
    }

    lineStart = lineEnd = lineStartSnapped = lineRoundedEnd = QPointF();
    update();
}
