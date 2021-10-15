#ifndef NODEFINDERSVGCONVERTER_H
#define NODEFINDERSVGCONVERTER_H

#include <QObject>
#include <QDomDocument>

#include <QHash>

#include "nodefinderelementclass.h"
#include "nodefinderelementwalker.h"

#include <ssplib/itemtypes.h>
#include "utils/nodefindereditingmodes.h"

class NodeFinderMgr;

class NodeFinderLabelModel;
class NodeFinderStationTracksModel;
class NodeFinderTurnoutModel;

class IObjectModel;

class QSvgRenderer;
class QIODevice;

class NodeFinderSVGConverter : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderSVGConverter(NodeFinderMgr *parent);

    QSvgRenderer *renderer() const;

    void clear();

    bool loadDocument(QIODevice *dev);
    bool save(QIODevice *dev);
    void reloadSVGRenderer();

    int calcDefaultTrackPenWidth();

    void loadLabelsAndTracks();

    void processElements();

    QDomElement elementById(const QString& id);

    void removeFakeIDs();
    void restoreFakeIDs();

    IObjectModel *getModel(EditingModes mode) const;

    inline NodeFinderElementWalker walkElements(const QStringList& tagOrder)
    {
        return NodeFinderElementWalker(tagOrder, elementClasses);
    }

    void removeCurrentSubElementFromItem();
    bool addCurrentElementToItem();

    ssplib::ItemBase *getCurItem() const;
    void setCurItem(ssplib::ItemBase *value);

    int getCurItemSubElemIdx() const;
    void setCurItemSubElemIdx(int value);

    inline ssplib::ElementPath getCurElementPath() const { return curElementPath; }

private:
    QString getFreeId_internal(const QString& base, int &counter);

    void renameElement(QDomElement &e, const QString &newId);

    void storeElement(QDomElement e);

    void removeElement(QDomElement e, bool *isFakeId = nullptr);

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

    bool parseLabel(QDomElement &e, QVector<ssplib::LabelItem>& labels);
    bool parsePlatform(QDomElement &e, QVector<ssplib::TrackItem>& platforms);
    bool parseTrackConnection(QDomElement &e, QVector<ssplib::TrackConnectionItem> &connections);

private:
    friend class NodeFinderElementClass;
    friend class NodeFinderMgr;
    friend class ElementSplitterHelper;

    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;

    QDomDocument mDoc;

    QHash<QString, NodeFinderElementClass> elementClasses;

    NodeFinderElementClass::ElementHash namedElements;
    NodeFinderElementClass::ElementHash fakeIds;

    NodeFinderLabelModel *labelsModel;
    NodeFinderStationTracksModel *tracksModel;
    NodeFinderTurnoutModel *turnoutModel;

    //Current selection
    NodeFinderElementWalker currentWalker;
    ssplib::ItemBase *curItem;
    int curItemSubElemIdx;
    ssplib::ElementPath curElementPath;
};

#endif // NODEFINDERSVGCONVERTER_H
