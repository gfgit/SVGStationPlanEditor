#ifndef SVGCREATORMANAGER_H
#define SVGCREATORMANAGER_H

#include <QObject>

#include <QVector>

class QIODevice;

class QGraphicsScene;
class SvgCreatorScene;

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

    struct GateTrack
    {
        int number;
        QGraphicsSimpleTextItem *trackLabelItem;
        QGraphicsLineItem *trackLineItem;
    };

    QVector<GateTrack> outTracks;
};

class SvgCreatorManager : public QObject
{
    Q_OBJECT
public:
    explicit SvgCreatorManager(QObject *parent = nullptr);

    void clear();
    bool loadStationXML(QIODevice *dev);

    QGraphicsScene *getScene() const;

private:
    PlatformItem createPlatform(const QString& name, int num);
    void movePlatformTo(PlatformItem& item, const QPointF& pos);

    GateItem createGate(QChar name, int outTrackCnt);
    void moveGateTo(GateItem& item, const QPointF& pos);

    void createStLabel();

private:
    friend class SvgCreatorScene;
    SvgCreatorScene *m_scene;

    StationLabel stLabel;

    QVector<GateItem> gates;
    QVector<PlatformItem> platforms;
};

#endif // SVGCREATORMANAGER_H
