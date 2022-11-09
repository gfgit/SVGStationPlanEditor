#ifndef SVGCREATORMANAGER_H
#define SVGCREATORMANAGER_H

#include <QObject>

#include <QVector>

class QIODevice;

class QGraphicsScene;
class SvgCreatorScene;

class QGraphicsItemGroup;
class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class QGraphicsLineItem;

struct StationLabel
{
    QGraphicsItemGroup *group = nullptr;
    QGraphicsSimpleTextItem *text = nullptr;
    QGraphicsRectItem *bgRect = nullptr;
    QString stationName;
};

struct PlatformItem
{
    QGraphicsItemGroup *group = nullptr;
    QGraphicsLineItem *lineItem = nullptr;
    QGraphicsRectItem *nameBgRect = nullptr;
    QGraphicsSimpleTextItem *nameText = nullptr;

    QString platfName;
    int platfNum = -1;
};

struct GateItem
{
    QGraphicsItemGroup *group = nullptr;
    QGraphicsSimpleTextItem *gateLabel = nullptr;
    QGraphicsRectItem *gateStationRect = nullptr;
    QChar gateLetter;

    struct GateTrack
    {
        int number = -1;
        QGraphicsSimpleTextItem *trackLabelItem = nullptr;
        QGraphicsLineItem *trackLineItem = nullptr;
    };

    QVector<GateTrack> outTracks;
};

struct TrackConnection
{
    QChar gateLetter;
    int gateTrackNumber = -1;
    QString platfName;
    int platfNumber = -1;
    bool plaftWestSide = false;
};

struct TrackConnectionItem
{
    QVector<TrackConnection> connections;
    QGraphicsLineItem *lineItem = nullptr;
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

    void addTrackConnection(const TrackConnectionItem& item);

private:
    friend class SvgCreatorScene;
    SvgCreatorScene *m_scene;

    StationLabel stLabel;

    QVector<GateItem> gates;
    QVector<PlatformItem> platforms;
    QVector<TrackConnectionItem> trackConnections;
};

#endif // SVGCREATORMANAGER_H
