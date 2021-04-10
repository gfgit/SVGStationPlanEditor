#ifndef NODEFINDERSVGCONVERTER_H
#define NODEFINDERSVGCONVERTER_H

#include <QObject>
#include <QDomDocument>

#include <QHash>

#include "nodefinderelementclass.h"

class NodeFinderMgr;
class NodeFinderLabelModel;
class NodeFinderStationTracksModel;
class QAbstractItemModel;

class QSvgRenderer;
class QIODevice;

class NodeFinderSVGConverter : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderSVGConverter(NodeFinderMgr *parent);

    QSvgRenderer *renderer() const;

    void clear();

    bool load(QIODevice *dev);
    bool save(QIODevice *dev);
    void reloadSVGRenderer();

    int calcDefaultTrackPenWidth();

    void loadLabelsAndTracks();

    void processElements();

    QDomElement elementById(const QString& id);

    void removeFakeIDs();
    void restoreFakeIDs();

    QAbstractItemModel *getLabelsModel() const;
    QAbstractItemModel *getTracksModel() const;

private:
    QString getFreeId_internal(const QString& base, int &counter);

    void renameElement(QDomElement &e, const QString &newId);

    void storeElement(QDomElement e);

    inline void registerClass(const QString& tagName)
    {
        NodeFinderElementClass c(tagName, tagName + '_');
        elementClasses.insert(tagName, c);
    }

private:
    void processDefs(QDomElement &g);
    void processGroup(QDomElement &g, int &generatedIdSerial, const QString &generatedIdBase);
    void processText(QDomElement &text, int &generatedIdSerial, const QString &generatedIdBase);
    void processTspan(QDomElement &tspan, QDomElement &text);
    void processInternalTspan(QDomElement &top, QDomElement &cur, QString &value);

private:
    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;

    QDomDocument mDoc;

    friend class NodeFinderElementClass;
    QHash<QString, NodeFinderElementClass> elementClasses;

    NodeFinderElementClass::ElementHash namedElements;
    NodeFinderElementClass::ElementHash fakeIds;

    friend class NodeFinderSVGWidget;
    NodeFinderLabelModel *labelsModel;
    NodeFinderStationTracksModel *tracksModel;
};

#endif // NODEFINDERSVGCONVERTER_H
