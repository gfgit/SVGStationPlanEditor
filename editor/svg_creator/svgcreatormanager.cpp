#include "svgcreatormanager.h"

#include <QGraphicsItem>
#include "model/svgcreatorscene.h"

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
    gates.clear();
    platforms.clear();
    trackConnections.clear();

    m_scene->clear();
}

bool SvgCreatorManager::loadStationXML(QIODevice *dev)
{
    ssplib::StationPlan plan;
    ssplib::StationInfoReader reader(&plan, dev);
    if (!reader.parse()) {
        return false;
    }

    clear();

    QRectF sceneRect(QPointF(), QSizeF(1000, 500));
    m_scene->setSceneRect(sceneRect);

    stLabel.stationName = plan.stationName;
    createStLabel();

    //Lay platforms parallel

    QRectF platfArea(sceneRect.left() + sceneRect.width() / 3,
                     sceneRect.top() + sceneRect.height() * 0.2,
                     sceneRect.width() / 3,
                     sceneRect.height() * 0.7);

    m_scene->addRect(platfArea, QPen(), Qt::green)->setZValue(-4);

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
        PlatformItem item = createPlatform(track.trackName, track.trackPos);
        movePlatformTo(item, platfPos);
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

    m_scene->addRect(gateArea, QPen(), Qt::yellow)->setZValue(-4);

    const double westGateDistance = gateArea.height() / westGateCount;
    const double eastGateDistance = gateArea.height() / eastGateCount;

    QPointF westGatePos = gateArea.center();
    westGatePos.ry() = gateArea.center().y() - westGateDistance * (westGateCount - 1) / 2;

    QPointF eastGatePos = gateArea.center();
    eastGatePos.setX(sceneRect.right() - eastGatePos.x());
    eastGatePos.ry() = gateArea.center().y() + eastGateDistance * (eastGateCount - 1) / 2;

    for (const auto &gate : qAsConst(plan.labels))
    {
        GateItem item = createGate(gate.gateLetter, gate.gateOutTrkCount);

        QPointF gatePos;
        if (gate.gateSide == ssplib::Side::West) {
            gatePos = westGatePos;
            westGatePos.ry() += westGateDistance; //Go down
        } else {
            gatePos = eastGatePos;
            eastGatePos.ry() -= eastGateDistance; //Go up
        }

        moveGateTo(item, gatePos);
        gates.append(item);
    }

    return true;
}

QGraphicsScene *SvgCreatorManager::getScene() const
{
    return m_scene;
}

PlatformItem SvgCreatorManager::createPlatform(const QString &name, int num)
{
    QFont font(QStringLiteral("sans-serif"), 12, QFont::Bold);

    QPen linePen(Qt::black, 5);
    linePen.setCapStyle(Qt::RoundCap);

    PlatformItem item;
    item.platfName = name;
    item.platfNum = num;

    QRectF sr = m_scene->sceneRect();
    item.left = m_scene->addLine(0, 0, sr.width() / 3, 0, linePen);

    item.nameBgRect = m_scene->addRect(QRectF(), QPen(), Qt::lightGray);
    item.nameText = m_scene->addSimpleText(item.platfName, font);
    item.nameText->setBrush(Qt::red);

    //Center text horizontally
    QRectF br = item.nameText->boundingRect();

    br.moveCenter(item.left->pos() + item.left->line().center());
    item.nameText->setPos(br.topLeft());

    br.adjust(-3, -3, 3, 3);
    item.nameBgRect->setRect(br);

    item.nameBgRect->setZValue(item.nameText->zValue() - 1);
    item.left->setZValue(item.nameBgRect->zValue() - 1);

    return item;
}

void SvgCreatorManager::movePlatformTo(PlatformItem &item, const QPointF &pos)
{
    item.left->setPos(pos);

    //Center text horizontally
    QRectF br = item.nameText->boundingRect();

    br.moveCenter(item.left->pos() + item.left->line().center());
    item.nameText->setPos(br.topLeft());

    br.adjust(-3, -3, 3, 3);
    item.nameBgRect->setRect(br);

    item.nameBgRect->setZValue(item.nameText->zValue() - 1);
}

const QLineF GateTrackLine(0, 0, 30, 0);

GateItem SvgCreatorManager::createGate(QChar name, int outTrackCnt)
{
    const QRectF labelRect(0, 0, 100, 50);

    QPen linePen(Qt::blue, 5);
    linePen.setCapStyle(Qt::RoundCap);

    QFont letterFont(QStringLiteral("sans-serif"), 12, QFont::Bold);
    QFont numberFont(QStringLiteral("sans-serif"), 9, QFont::Normal);

    GateItem item;
    item.gateLetter = name;

    item.gateLabel = m_scene->addSimpleText(item.gateLetter, letterFont);
    item.gateStationRect = m_scene->addRect(labelRect, QPen(), Qt::lightGray);

    for(int i = 0; i < outTrackCnt; i++)
    {
        GateItem::GateTrack track;
        track.number = i;
        track.trackLabelItem = m_scene->addSimpleText(QString::number(track.number), numberFont);
        track.trackLineItem = m_scene->addLine(GateTrackLine);
        track.trackLineItem->setPen(linePen);

        item.outTracks.append(track);
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

    if (!stLabel.bgRect)
        stLabel.bgRect = m_scene->addRect(QRectF(), QPen(), Qt::gray);

    if (!stLabel.text)
        stLabel.text = m_scene->addSimpleText(QString(), font);

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

void SvgCreatorManager::addTrackConnection(const TrackConnectionItem &item)
{
    trackConnections.append(item);
}
