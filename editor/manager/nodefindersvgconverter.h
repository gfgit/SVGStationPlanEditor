#ifndef NODEFINDERSVGCONVERTER_H
#define NODEFINDERSVGCONVERTER_H

#include <QObject>
#include <QDomDocument>

#include <QHash>

#include "nodefinderelementclass.h"
#include "nodefinderelementwalker.h"

#include "utils/nodefinderutils.h"

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

    inline NodeFinderElementWalker walkElements(const QStringList& tagOrder)
    {
        return NodeFinderElementWalker(tagOrder, elementClasses);
    }

    void removeCurrentSubElementFromItem();
    bool addCurrentElementToItem();

    ItemBase *getCurItem() const;
    void setCurItem(ItemBase *value);

    int getCurItemSubElemIdx() const;
    void setCurItemSubElemIdx(int value);

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
    friend class NodeFinderElementClass;
    friend class NodeFinderSVGWidget;
    friend class NodeFinderMgr;

    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;

    QDomDocument mDoc;

    QHash<QString, NodeFinderElementClass> elementClasses;

    NodeFinderElementClass::ElementHash namedElements;
    NodeFinderElementClass::ElementHash fakeIds;

    NodeFinderLabelModel *labelsModel;
    NodeFinderStationTracksModel *tracksModel;

    //Current selection
    NodeFinderElementWalker currentWalker;
    ItemBase *curItem;
    int curItemSubElemIdx;
    ElementPath curElementPath;
};

#endif // NODEFINDERSVGCONVERTER_H
