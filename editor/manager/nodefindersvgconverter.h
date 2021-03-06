#ifndef NODEFINDERSVGCONVERTER_H
#define NODEFINDERSVGCONVERTER_H

#include <QObject>
#include <QDomDocument>

#include <QHash>

#include "nodefinderelementclass.h"
#include "nodefinderelementwalker.h"

#include <ssplib/itemtypes.h>
#include <ssplib/parsing/editinginfo.h>
#include <ssplib/stationplan.h>

#include "utils/nodefindereditingmodes.h"

class NodeFinderMgr;

class NodeFinderLabelModel;
class NodeFinderStationTracksModel;
class NodeFinderTurnoutModel;

class IObjectModel;

class QSvgRenderer;
class QIODevice;

class QAbstractItemDelegate;

class NodeFinderSVGConverter : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderSVGConverter(NodeFinderMgr *parent);

    QSvgRenderer *renderer() const;

    void clear();

    bool loadXML(QIODevice *dev);
    void clearXML();

    bool loadDocument(QIODevice *dev);
    bool save(QIODevice *dev);
    void reloadSVGRenderer();

    int calcDefaultTrackPenWidth();

    void processElements();

    QDomElement elementById(const QString& id);

    void removeFakeIDs();
    void restoreFakeIDs();

    IObjectModel *getModel(EditingModes mode) const;

    QAbstractItemDelegate *getDelegateFor(int col, EditingModes mode, QObject *parent, bool &outShowCol) const;

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

    inline bool hasXML() const { return m_hasXML; }

private:
    void renameElement(QDomElement &e, const QString &newId);

    void storeElement(QDomElement e);

    void removeElement(QDomElement e, bool *isFakeId = nullptr);

    inline void registerClass(const QString& tagName)
    {
        NodeFinderElementClass c(tagName, tagName + '_');
        elementClasses.insert(tagName, c);
    }

private:
    friend class NodeFinderElementClass;
    friend class NodeFinderMgr;
    friend class ElementSplitterHelper;

    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;

    QDomDocument mDoc;
    ssplib::StationPlan m_plan;
    ssplib::StationPlan m_xmlPlan;
    ssplib::EditingInfo m_info;

    QHash<QString, NodeFinderElementClass> elementClasses;

    NodeFinderElementClass::ElementHash fakeIds;

    NodeFinderLabelModel *labelsModel;
    NodeFinderStationTracksModel *tracksModel;
    NodeFinderTurnoutModel *turnoutModel;

    //Current selection
    NodeFinderElementWalker currentWalker;
    ssplib::ItemBase *curItem;
    int curItemSubElemIdx;
    ssplib::ElementPath curElementPath;

    bool m_hasXML;
};

#endif // NODEFINDERSVGCONVERTER_H
