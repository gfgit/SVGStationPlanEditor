#include "svgcreatormanager.h"

#include <QGraphicsItem>
#include "model/svgcreatorscene.h"
#include "model/svgconnectionsmodel.h"

#include "model/svgcreatorsvgwriter.h"
#include "ssplib/parsing/stationinfoparser.h"
#include "ssplib/stationplan.h"

#include <QDebug>

SvgCreatorManager::SvgCreatorManager(QObject *parent) :
    QObject{parent}
{
    m_scene = new SvgCreatorScene(this);
}

void SvgCreatorManager::clear()
{
    stLabel = StationLabel();

    qDeleteAll(gates);
    gates.clear();

    qDeleteAll(platforms);
    platforms.clear();

    qDeleteAll(trackConnections);
    trackConnections.clear();

    m_scene->clear();
}

bool SvgCreatorManager::loadStationXML(QIODevice *dev)
{
    ssplib::StationPlan plan;
    ssplib::StationInfoReader reader(&plan, dev);
    if (!reader.parse())
        return false;

    clear();

    QRectF sceneRect(QPointF(), QSizeF(1500, 500));
    m_scene->setSceneRect(sceneRect);

    stLabel.stationName = plan.stationName;
    createStLabel();

    //Lay platforms parallel

    QRectF platfArea(sceneRect.left() + sceneRect.width() / 3,
                     sceneRect.top() + sceneRect.height() * 0.2,
                     sceneRect.width() / 3,
                     sceneRect.height() * 0.7);

    const double platfDistance = platfArea.height() / plan.platforms.size();
    QPointF platfPos = platfArea.topLeft();
    platfPos.ry() = platfArea.center().y() - platfDistance * (plan.platforms.size() - 1) / 2;

    std::sort(plan.platforms.begin(),
              plan.platforms.end(),
              [](const ssplib::TrackItem &lhs, const ssplib::TrackItem &rhs) {
                  return lhs.trackPos > rhs.trackPos;
              });

    for (const auto &track : qAsConst(plan.platforms))
    {
        PlatformItem *item = createPlatform(track.trackName, track.trackPos);
        movePlatformTo(*item, platfPos);
        platforms.append(item);

        platfPos.ry() += platfDistance;
    }

    //Lay gates
    std::sort(plan.labels.begin(),
              plan.labels.end(),
              [](const ssplib::LabelItem &lhs, const ssplib::LabelItem &rhs) {
                  return lhs.gateLetter < rhs.gateLetter;
              });

    //Calculate spacing
    int westGateCount = 0;
    for (const auto &label : qAsConst(plan.labels))
    {
        if (label.gateSide == ssplib::Side::West)
            westGateCount++;
    }
    int eastGateCount = plan.labels.size() - westGateCount;

    QRectF gateArea(sceneRect.left(),
                    sceneRect.top() + sceneRect.height() * 0.15,
                    sceneRect.width() / 7,
                    sceneRect.height() * 0.8);

    const double westGateDistance = gateArea.height() / westGateCount;
    const double eastGateDistance = gateArea.height() / eastGateCount;

    QPointF westGatePos = gateArea.center();
    westGatePos.ry() = gateArea.center().y() - westGateDistance * (westGateCount - 1) / 2;

    QPointF eastGatePos = gateArea.center();
    eastGatePos.setX(sceneRect.right() - eastGatePos.x());
    eastGatePos.ry() = gateArea.center().y() + eastGateDistance * (eastGateCount - 1) / 2;

    for (const auto &gate : qAsConst(plan.labels))
    {
        GateItem *item = createGate(gate.gateLetter, gate.gateOutTrkCount);

        QPointF gatePos;
        if (gate.gateSide == ssplib::Side::West) {
            gatePos = westGatePos;
            westGatePos.ry() += westGateDistance; //Go down
        } else {
            gatePos = eastGatePos;
            eastGatePos.ry() -= eastGateDistance; //Go up
        }

        moveGateTo(*item, gatePos);
        gates.append(item);
    }

    return true;
}

bool SvgCreatorManager::saveSVG(QIODevice *dev)
{
    SvgCreatorSVGWriter writer(this);
    return writer.writeSVG(dev);
}

QGraphicsScene *SvgCreatorManager::getScene() const
{
    return m_scene;
}

QAbstractItemModel *SvgCreatorManager::getConnectionsModel() const
{
    return m_scene->getConnModel();
}

PlatformItem* SvgCreatorManager::createPlatform(const QString &name, int num)
{
    QFont font(QStringLiteral("sans-serif"), 12, QFont::Bold);

    QPen linePen(Qt::black, 5);
    linePen.setCapStyle(Qt::RoundCap);

    PlatformItem *item = new PlatformItem;
    item->platfName = name;
    item->platfNum = num;

    QRectF sr = m_scene->sceneRect();
    item->lineItem = m_scene->addLine(0, 0, sr.width() / 3, 0, linePen);
    item->lineItem->setData(GraphicsItemTypeKey, int(GraphicsItemType::Platform));

    item->nameBgRect = m_scene->addRect(QRectF(), QPen(), Qt::lightGray);
    item->nameText = m_scene->addSimpleText(item->platfName, font);
    item->nameText->setBrush(Qt::red);

    item->group = m_scene->createItemGroup({item->nameText, item->nameBgRect, item->lineItem});
    item->group->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);

    //Center text horizontally
    QRectF br = item->nameText->boundingRect();

    br.moveCenter(item->lineItem->pos() + item->lineItem->line().center());
    item->nameText->setPos(br.topLeft());

    br.adjust(-3, -3, 3, 3);
    item->nameBgRect->setRect(br);

    item->nameBgRect->setZValue(item->nameText->zValue() - 1);
    item->lineItem->setZValue(item->nameBgRect->zValue() - 1);

    return item;
}

void SvgCreatorManager::movePlatformTo(PlatformItem &item, const QPointF &pos)
{
    item.lineItem->setPos(pos);

    //Center text horizontally
    QRectF br = item.nameText->boundingRect();

    br.moveCenter(item.lineItem->pos() + item.lineItem->line().center());
    item.nameText->setPos(br.topLeft());

    br.adjust(-3, -3, 3, 3);
    item.nameBgRect->setRect(br);

    item.nameBgRect->setZValue(item.nameText->zValue() - 1);
}

const QLineF GateTrackLine(0, 0, 30, 0);

GateItem* SvgCreatorManager::createGate(QChar name, int outTrackCnt)
{
    const QRectF labelRect(0, 0, 100, 50);

    QPen linePen(Qt::blue, 5);
    linePen.setCapStyle(Qt::RoundCap);

    QFont letterFont(QStringLiteral("sans-serif"), 12, QFont::Bold);
    QFont numberFont(QStringLiteral("sans-serif"), 9, QFont::Normal);

    GateItem *item = new GateItem;
    item->gateLetter = name;

    item->gateLabel = m_scene->addSimpleText(item->gateLetter, letterFont);
    item->gateStationRect = m_scene->addRect(labelRect, QPen(), Qt::lightGray);
    item->group = m_scene->createItemGroup({item->gateLabel, item->gateStationRect});
    item->group->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);

    for(int i = 0; i < outTrackCnt; i++)
    {
        GateItem::GateTrack track;
        track.number = i;
        track.trackLabelItem = m_scene->addSimpleText(QString::number(track.number), numberFont);
        track.trackLineItem = m_scene->addLine(GateTrackLine);
        track.trackLineItem->setPen(linePen);

        track.trackLineItem->setData(GraphicsItemTypeKey, int(GraphicsItemType::GateTrack));

        item->group->addToGroup(track.trackLabelItem);
        item->group->addToGroup(track.trackLineItem);

        item->outTracks.append(track);
    }

    return item;
}

void SvgCreatorManager::moveGateTo(GateItem &item, const QPointF &pos)
{
    QRectF br = item.gateStationRect->rect();
    br.moveCenter(pos);
    item.gateStationRect->setRect(br);

    //Align text centered horizontaly on top of rect
    QRectF textBr = item.gateLabel->boundingRect();
    textBr.moveCenter(QPointF(br.center().x(), br.top() - textBr.height() / 2));
    item.gateLabel->setPos(textBr.topLeft());

    bool isWestSide = pos.x() < m_scene->sceneRect().center().x();
    const int side = isWestSide ? 1 : -1;

    const double trackLabelOffset = 3;
    const double trackLabelSize = 15;

    const double trackDistance = 30;

    double trkPosX = isWestSide ? br.right() : br.left();
    trkPosX += trackLabelOffset * side;

    double trkPosY = pos.y() - trackDistance * (item.outTracks.size() - 1) / 2;

    double trkLineOffset = (trackLabelSize) * side;
    if(!isWestSide)
        trkLineOffset -= GateTrackLine.length(); //Track line length

    for(auto &track : item.outTracks)
    {
        QRectF labelBr = track.trackLabelItem->boundingRect();
        labelBr.moveCenter(QPointF(trkPosX + trackLabelSize / 2 * side, trkPosY));
        track.trackLabelItem->setPos(labelBr.topLeft());
        track.trackLineItem->setPos(trkPosX + trkLineOffset, trkPosY);

        trkPosY += trackDistance;
    }
}

void SvgCreatorManager::createStLabel()
{
    QFont font(QStringLiteral("sans-serif"), 20, QFont::Bold);

    if(!stLabel.group)
    {
        stLabel.group = new QGraphicsItemGroup;
        stLabel.group->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        m_scene->addItem(stLabel.group);
    }

    if (!stLabel.bgRect)
    {
        stLabel.bgRect = m_scene->addRect(QRectF(), QPen(), Qt::gray);
        stLabel.group->addToGroup(stLabel.bgRect);
    }

    if (!stLabel.text)
    {
        stLabel.text = m_scene->addSimpleText(QString(), font);
        stLabel.group->addToGroup(stLabel.text);
    }

    stLabel.text->setText(stLabel.stationName);
    stLabel.bgRect->setZValue(stLabel.text->zValue() - 1);

    //Center text horizontally
    QRectF br = stLabel.text->boundingRect();
    QRectF sr = m_scene->sceneRect();

    br.moveCenter(QPointF(sr.center().x(), sr.top() + sr.height() / 10));
    stLabel.text->setPos(br.topLeft());

    br.adjust(-5, -5, 5, 5);
    stLabel.bgRect->setRect(br);
}

void SvgCreatorManager::addTrackConnection(TrackConnectionItem *item)
{
    trackConnections.append(item);
    item->lineItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    item->lineItem->setData(GraphicsItemTypeKey, int(GraphicsItemType::ConnectionTrack));
}

SvgTrackItemSplitter::SvgTrackItemSplitter(SvgCreatorManager *mgr) :
    manager(mgr),
    m_item(nullptr),
    currentIndex(-1)
{

}

SvgTrackItemSplitter::~SvgTrackItemSplitter()
{
    clearOverlay();
}

inline QLineF mapLineToScene(QGraphicsItem *item, const QLineF& l)
{
    return QLineF(item->mapToScene(l.p1()),
                  item->mapToScene(l.p2()));
}

inline QLineF mapLineFromScene(QGraphicsItem *item, const QLineF& l)
{
    return QLineF(item->mapFromScene(l.p1()),
                  item->mapFromScene(l.p2()));
}

void SvgTrackItemSplitter::setItem(TrackConnectionItem *item)
{
    m_item = item;
    calculateIntersections();
}

QPointF SvgTrackItemSplitter::getCurrentPoint() const
{
    if(currentIndex < m_intersections.size())
        return m_intersections.at(currentIndex).intersection;
    return QPointF();
}

bool SvgTrackItemSplitter::applyIntersection(bool skip)
{
    if(!m_item || currentIndex >= m_intersections.size())
        return false;

    const bool isLastSegment = currentIndex == m_intersections.size() - 1;

    if(skip)
    {
        currentIndex++;

        if(isLastSegment)
            return false; //We split last one

        drawIntersection();
        return true;
    }

    auto scene = static_cast<SvgCreatorScene*>(manager->getScene());
    const Entry& entry = m_intersections.at(currentIndex);

    //Split
    const bool isOtherEdge = (entry.otherLine.p1() == entry.intersection) ||
                             (entry.otherLine.p2() == entry.intersection);
    if(!isOtherEdge)
    {
        TrackConnectionItem *otherSplit = new TrackConnectionItem;
        otherSplit->connections = entry.otherItem->connections;
        otherSplit->lineItem = scene->addLine(QLineF(entry.otherLine.p1(), entry.intersection),
                                            entry.otherItem->lineItem->pen());

        QLineF otherRemaining(entry.intersection, entry.otherLine.p2());
        entry.otherItem->lineItem->setLine(mapLineFromScene(entry.otherItem->lineItem, otherRemaining));

        manager->addTrackConnection(otherSplit);
    }

    const bool isOurEdge = (remainingLine.p1() == entry.intersection) ||
                           (remainingLine.p2() == entry.intersection);

    if(!isOurEdge || (isLastSegment && !remainingLine.isNull()))
    {
        QLineF segment(remainingLine.p1(), entry.intersection);
        if(isLastSegment && isOurEdge)
            segment = remainingLine; //Last segment with edge takes all remaining line
        remainingLine.setP1(segment.p2());

        if(segment.p1() == originalLine.p1())
        {
            //Assign first segment to original item
            QLineF segmentMapped = mapLineFromScene(m_item->lineItem, segment);
            m_item->lineItem->setLine(segmentMapped);
        }
        else
        {
            TrackConnectionItem *ourSplit = new TrackConnectionItem;
            ourSplit->connections = m_item->connections;
            ourSplit->lineItem = scene->addLine(segment, m_item->lineItem->pen());
            manager->addTrackConnection(ourSplit);
        }
    }

    if(isLastSegment && !remainingLine.isNull())
    {
        TrackConnectionItem *ourSplit = new TrackConnectionItem;
        ourSplit->connections = m_item->connections;
        ourSplit->lineItem = scene->addLine(remainingLine, m_item->lineItem->pen());
        manager->addTrackConnection(ourSplit);
    }

    //Increment index
    currentIndex++;

    if(isLastSegment)
        return false; //We split last one

    drawIntersection();
    return true;
}

void SvgTrackItemSplitter::clearOverlay()
{
    static_cast<SvgCreatorScene*>(manager->getScene())->clearOverlayLines();
}

void SvgTrackItemSplitter::calculateIntersections()
{
    m_intersections.clear();
    originalLine = remainingLine = QLineF();
    currentIndex = -1;

    if(!m_item)
        return;

    originalLine = mapLineToScene(m_item->lineItem, m_item->lineItem->line());
    remainingLine = originalLine;

    //Check collisions
    for(TrackConnectionItem* other : qAsConst(manager->trackConnections))
    {
        if(m_item == other)
            continue; //Skip ourselves

        Entry entry;
        entry.otherItem = other;
        entry.otherLine = mapLineToScene(other->lineItem, other->lineItem->line());

        QLineF::IntersectionType res = entry.otherLine.intersects(remainingLine, &entry.intersection);
        if(res != QLineF::BoundedIntersection)
            continue;

        entry.distance = (originalLine.p1() - entry.intersection).manhattanLength();

        const bool isOurEdge = (remainingLine.p1() == entry.intersection) ||
                               (remainingLine.p2() == entry.intersection);

        const bool isOtherEdge = (entry.otherLine.p1() == entry.intersection) ||
                               (entry.otherLine.p2() == entry.intersection);

        if(isOurEdge && isOtherEdge)
            continue; //Skip because item are simply concatenated, no split needed

        m_intersections.append(entry);
    }

    if(m_intersections.size())
    {
        //Sort by distance from start point
        std::sort(m_intersections.begin(), m_intersections.end(),
                  [](const Entry& lhs, const Entry& rhs)
                  {
                      return lhs.distance < rhs.distance;
                  });

        currentIndex = 0;
        drawIntersection();
    }
}

void SvgTrackItemSplitter::drawIntersection()
{
    auto scene = static_cast<SvgCreatorScene*>(manager->getScene());
    const Entry& entry = m_intersections.at(currentIndex);

    scene->setOverlayLines(remainingLine, entry.otherLine);
}
