#ifndef SVGCREATORMANAGER_H
#define SVGCREATORMANAGER_H

#include <QObject>

#include <QVector>
#include <QLineF>

#include <utils/svg_trackconn_util.h>

class QIODevice;

class QGraphicsScene;
class SvgCreatorScene;

class QGraphicsItemGroup;
class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class QGraphicsLineItem;

class QAbstractItemModel;

enum class GraphicsItemType
{
    Unknown = 0,
    Platform,
    GateTrack,
    ConnectionTrack
};

constexpr int GraphicsItemTypeKey = 1;
constexpr int GraphicsOldPenKey = 2;

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

struct TrackConnectionItem
{
    QVector<ssplib::TrackConnectionInfo> connections;
    QGraphicsLineItem *lineItem = nullptr;
};

class SvgCreatorManager : public QObject
{
    Q_OBJECT
public:
    explicit SvgCreatorManager(QObject *parent = nullptr);

    void clear();
    bool loadStationXML(QIODevice *dev, bool invertTracks);
    bool saveSVG(QIODevice *dev);

    QGraphicsScene *getScene() const;

    QAbstractItemModel *getConnectionsModel() const;

    inline QString getStName() const { return stLabel.stationName; }

signals:
    void splitTrackRequested(TrackConnectionItem *item, bool silent = false);

private:
    PlatformItem *createPlatform(const QString& name, int num, double width);
    void movePlatformTo(PlatformItem& item, const QPointF& pos);

    GateItem *createGate(QChar name, int outTrackCnt, bool isWest);
    void moveGateTo(GateItem& item, const QPointF& pos);

    void createStLabel();

    void addTrackConnection(TrackConnectionItem *item);

private:
    friend class SvgTrackItemSplitter;
    friend class SvgCreatorScene;
    friend class SvgCreatorSVGWriter;
    SvgCreatorScene *m_scene;

    StationLabel stLabel;

    QVector<GateItem *> gates;
    QVector<PlatformItem *> platforms;
    QVector<TrackConnectionItem *> trackConnections;
};

class SvgTrackItemSplitter
{
public:
    SvgTrackItemSplitter(SvgCreatorManager *mgr);
    ~SvgTrackItemSplitter();

    void setItem(TrackConnectionItem *item);

    inline int getIntersectionCount() const { return m_intersections.size(); }

    int getCurrentIndex() const { return currentIndex; }

    QPointF getCurrentPoint() const;

    bool applyIntersection(bool skip = false);

    void clearOverlay();

private:
    void calculateIntersections();
    void drawIntersection();

private:
    SvgCreatorManager *manager;
    TrackConnectionItem *m_item;

    struct Entry
    {
        TrackConnectionItem *otherItem;
        QLineF otherLine;
        QPointF intersection;
        double distance;
    };
    QVector<Entry> m_intersections;

    QLineF originalLine;
    QLineF remainingLine;
    int currentIndex;
};

#endif // SVGCREATORMANAGER_H
