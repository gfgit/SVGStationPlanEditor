#ifndef SVGCREATORSCENE_H
#define SVGCREATORSCENE_H

#include <QObject>

#include <QVector>

class QIODevice;

class QGraphicsScene;
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

class SvgCreatorScene : public QObject
{
    Q_OBJECT
public:
    explicit SvgCreatorScene(QObject *parent = nullptr);

    void clear();
    bool loadStationXML(QIODevice *dev);

    inline QGraphicsScene *getScene() const { return m_scene; }

private slots:
    void onSelectionChanged();

    PlatformItem createPlatform(const QString& name, int num);
    void movePlatformTo(PlatformItem& item, const QPointF& pos);

    GateItem createGate(QChar name, int outTrackCnt);
    void moveGateTo(GateItem& item, const QPointF& pos);

private:
    void createStLabel();

private:
    QGraphicsScene *m_scene;
    QGraphicsRectItem *m_bkItem;

    StationLabel stLabel;

    QVector<GateItem> gates;
    QVector<PlatformItem> platforms;
};

#endif // SVGCREATORSCENE_H
