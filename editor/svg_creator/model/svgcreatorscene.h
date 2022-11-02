#ifndef SVGCREATORSCENE_H
#define SVGCREATORSCENE_H

#include <QObject>

#include <QVector>

class QGraphicScene;
class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class QGraphicsLineItem;

struct StationLabel
{
    QGraphicsSimpleTextItem *text = nullptr;
    QGraphicsRectItem *bgRect = nullptr;
    QString stationName;
};

struct PlatformItem
{
    QGraphicsLineItem *left = nullptr;
    QGraphicsLineItem *right = nullptr;
    QGraphicsRectItem *nameBgRect = nullptr;
    QGraphicsSimpleTextItem *nameText = nullptr;
    QGraphicsRectItem *platfEndRect = nullptr;
    QString platfName;
    int platfNum;
};

struct GateItem
{
    QGraphicsSimpleTextItem *gateLabel;
    QGraphicsRectItem *gateStationRect;
    QChar gateLetter;
};

class SvgCreatorScene : public QObject
{
    Q_OBJECT
public:
    explicit SvgCreatorScene(QObject *parent = nullptr);

private:
    QGraphicScene *m_scene;

    StationLabel *stLabel;

    QVector<GateItem> gates;
    QVector<PlatformItem> platforms;
};

#endif // SVGCREATORSCENE_H
