#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

#include "model/nodefinderlabelmodel.h"
#include "model/nodefinderstationtracksmodel.h"
#include "model/nodefinderturnoutmodel.h"

#include "utils/comboboxdelegate.h"
#include "utils/completiondelegate.h"

#include <ssplib/utils/svg_path_utils.h>
#include <ssplib/utils/svg_constants.h>

#include <ssplib/parsing/domparser.h>
#include <ssplib/parsing/stationinfoparser.h>

#include <QSvgRenderer>

#include <QTemporaryFile>

#include <QDebug>

NodeFinderSVGConverter::NodeFinderSVGConverter(NodeFinderMgr *parent) :
    QObject(parent),
    nodeMgr(parent),
    curItem(nullptr),
    curItemSubElemIdx(-1),
    m_hasXML(false)
{
    registerClass(ssplib::svg_tags::GroupTag); //Groups
    registerClass(ssplib::svg_tags::RectTag);
    registerClass(ssplib::svg_tags::PathTag);
    registerClass(ssplib::svg_tags::LineTag);
    registerClass(ssplib::svg_tags::PolylineTag);

    mSvg = new QSvgRenderer(this);

    labelsModel = new NodeFinderLabelModel(&m_plan, &m_xmlPlan, nodeMgr, this);
    tracksModel = new NodeFinderStationTracksModel(&m_plan, &m_xmlPlan, nodeMgr, this);
    turnoutModel = new NodeFinderTurnoutModel(&m_plan, &m_xmlPlan, nodeMgr, this);

    //Connect models to keep views updated
    connect(labelsModel, &NodeFinderLabelModel::labelsChanged, turnoutModel, &NodeFinderTurnoutModel::refreshData);
    connect(tracksModel, &NodeFinderStationTracksModel::tracksChanged, turnoutModel, &NodeFinderTurnoutModel::refreshData);

    m_info.setCallback([this](QDomElement &e) { storeElement(e); });
}

QSvgRenderer *NodeFinderSVGConverter::renderer() const
{
    return mSvg;
}

void NodeFinderSVGConverter::clear()
{
    for(NodeFinderElementClass &c : elementClasses)
        c.clear();

    fakeIds.clear();
    fakeIds.squeeze();

    m_plan.clear();
    m_xmlPlan.clear();
    m_info.clear();

    m_hasXML = false;

    currentWalker = NodeFinderElementWalker();
    curItem = nullptr;
    curItemSubElemIdx = -1;
    curElementPath = ssplib::ElementPath();

    mDoc.clear();
}

bool NodeFinderSVGConverter::loadXML(QIODevice *dev)
{
    m_xmlPlan.clear();

    m_hasXML = false;

    ssplib::StationInfoReader reader(&m_xmlPlan, dev);
    if(!reader.parse())
    {
        clearXML();
        return false;
    }

    //Create missing items and mirror data
    for(const ssplib::LabelItem& gate : std::as_const(m_xmlPlan.labels))
    {
        bool found = false;
        for(int i = 0; i < m_plan.labels.size(); i++)
        {
            ssplib::LabelItem& item = m_plan.labels[i];
            if(gate.gateLetter == item.gateLetter)
            {
                //Found, merge info
                found = true;
                item.gateOutTrkCount = gate.gateOutTrkCount;
                item.gateSide = gate.gateSide;
            }
            else if(gate.labelText == item.labelText)
            {
                //Clear name because it's duplicate
                item.labelText.clear();
            }
        }

        if(!found)
        {
            //Item was missing, add it
            m_plan.labels.append(gate);
        }
    }

    for(const ssplib::TrackItem& track : std::as_const(m_xmlPlan.platforms))
    {
        bool found = false;
        for(int i = 0; i < m_plan.platforms.size(); i++)
        {
            ssplib::TrackItem& item = m_plan.platforms[i];
            if(track.trackPos == item.trackPos)
            {
                //Found, merge info
                found = true;
                item.trackName = track.trackName;
            }
            else if(track.trackName == item.trackName)
            {
                //Clear name because it's duplicate
                item.trackName.clear();
            }
        }

        if(!found)
        {
            //Item was missing, add it
            m_plan.platforms.append(track);
        }
    }

    for(const ssplib::TrackConnectionItem& track : std::as_const(m_xmlPlan.trackConnections))
    {
        bool found = false;
        for(int i = 0; i < m_plan.trackConnections.size(); i++)
        {
            ssplib::TrackConnectionItem& item = m_plan.trackConnections[i];
            if(track.info.matchNames(item.info))
            {
                //Found, merge info
                found = true;
                break;
            }
        }

        if(!found)
        {
            //Item was missing, add it
            m_plan.trackConnections.append(track);
        }
    }

    m_hasXML = true;

    //Refresh models
    labelsModel->refreshModel();
    tracksModel->refreshModel();
    turnoutModel->refreshModel();

    return true;
}

void NodeFinderSVGConverter::clearXML()
{
    m_xmlPlan.clear();

    m_hasXML = false;

    //Refresh models
    labelsModel->refreshModel();
    tracksModel->refreshModel();
    turnoutModel->refreshModel();
}

bool NodeFinderSVGConverter::loadDocument(QIODevice *dev)
{
    //FIXME: error reporting
    clear();

    QXmlStreamReader xml(dev);
    xml.setNamespaceProcessing(false);

    bool ret = mDoc.setContent(&xml, xml.namespaceProcessing());

    return ret;
}

bool NodeFinderSVGConverter::save(QIODevice *dev)
{
    const int IndentSize = 1;

    removeFakeIDs();

    //Save contents
    QTextStream stream(dev);
    mDoc.save(stream, IndentSize);

    restoreFakeIDs();

    return true;
}

void NodeFinderSVGConverter::reloadSVGRenderer()
{
    QTemporaryFile file;
    if(!file.open())
        return;

    if(!save(&file))
        return;

    file.reset();

    QXmlStreamReader xml(&file);
    mSvg->load(&xml);
}

int NodeFinderSVGConverter::calcDefaultTrackPenWidth()
{
    QSize sz = mSvg->viewBox().size();
    int trackPenWidth = qMin(sz.width(), sz.height()) / 100;
    if(trackPenWidth < 2)
        trackPenWidth = 2;
    return trackPenWidth;
}

void NodeFinderSVGConverter::processElements()
{
    ssplib::DOMParser parser(&mDoc, &m_plan, &m_info);
    parser.parse();

    //Sort items
    std::sort(m_plan.labels.begin(), m_plan.labels.end());
    std::sort(m_plan.platforms.begin(), m_plan.platforms.end());
    std::sort(m_plan.trackConnections.begin(), m_plan.trackConnections.end());

    //Refresh models
    labelsModel->refreshModel();
    tracksModel->refreshModel();
    turnoutModel->refreshModel();
}

QDomElement NodeFinderSVGConverter::elementById(const QString &id)
{
    auto it = m_info.namedElements.constFind(id);
    if(it != m_info.namedElements.constEnd())
        return it.value();

    auto it2 = fakeIds.constFind(id);
    if(it2 != fakeIds.constEnd())
        return it2.value();

    //Not found
    return QDomElement();
}

void NodeFinderSVGConverter::removeFakeIDs()
{
    //Remove unused generated IDs
    for(QDomElement& e : fakeIds)
    {
        e.removeAttribute(ssplib::svg_attr::ID);
    }
}

void NodeFinderSVGConverter::restoreFakeIDs()
{
    //Restore generated IDs
    for(auto it = fakeIds.begin(); it != fakeIds.end(); it++)
    {
        it.value().setAttribute(ssplib::svg_attr::ID, it.key());
    }
}

IObjectModel *NodeFinderSVGConverter::getModel(EditingModes mode) const
{
    switch (mode)
    {
    case EditingModes::LabelEditing:
        return labelsModel;
    case EditingModes::StationTrackEditing:
        return tracksModel;
    case EditingModes::TrackPathEditing:
        return turnoutModel;
    default:
        break;
    }

    return nullptr;
}

QAbstractItemDelegate *NodeFinderSVGConverter::getDelegateFor(int col, EditingModes mode, QObject *parent, bool &outShowCol) const
{
    outShowCol = true;

    switch (mode)
    {
    case EditingModes::LabelEditing:
    {
        if(col == NodeFinderLabelModel::GateSideCol)
        {
            QStringList list = {IObjectModel::getTrackSideName(ssplib::Side::West),
                                IObjectModel::getTrackSideName(ssplib::Side::East)};
            return new ComboboxDelegate(list, parent);
        }
        break;
    }
    case EditingModes::StationTrackEditing:
    {
        if(col == NodeFinderStationTracksModel::SpecialMixedColumn)
            outShowCol = false; //Hide column to the user
        break;
    }
    case EditingModes::TrackPathEditing:
    {
        if(col == NodeFinderTurnoutModel::StationTrackSideCol)
        {
            QStringList list = {IObjectModel::getTrackSideName(ssplib::Side::West),
                                IObjectModel::getTrackSideName(ssplib::Side::East)};
            return new ComboboxDelegate(list, parent);
        }
        if(col == NodeFinderTurnoutModel::StationTrackCol)
        {
            CompletionDelegate *delegate = new CompletionDelegate(tracksModel, parent);
            delegate->setColumn(NodeFinderStationTracksModel::SpecialMixedColumn);
            return delegate;
        }
        break;
    }
    default:
        break;
    }
    return nullptr;
}

void NodeFinderSVGConverter::removeCurrentSubElementFromItem()
{
    if(!curItem || curItemSubElemIdx < 0 || curItemSubElemIdx >= curItem->elements.size())
        return;

    IObjectModel *model = getModel(nodeMgr->mode());
    if(!model)
        return;

    model->removeElementFromItem(curItem, curItemSubElemIdx);
    curItemSubElemIdx = -1; //Reset sub element selection
}

bool NodeFinderSVGConverter::addCurrentElementToItem()
{
    if(!currentWalker.isValid() || !curItem)
        return false;

    IObjectModel *model = getModel(nodeMgr->mode());
    if(!model)
        return false;

    ssplib::ElementPath path;
    path.elem = currentWalker.element();
    if(!ssplib::utils::convertElementToPath(path.elem, path.path))
        return false;

    path.strokeWidth = 0;
    const QRectF bounds = path.path.boundingRect();
    if(!ssplib::utils::parseStrokeWidthRecursve(path.elem, bounds, path.strokeWidth))
        path.strokeWidth = 0;

    return model->addElementToItem(path, curItem);
}

void NodeFinderSVGConverter::renameElement(QDomElement &e, const QString &newId)
{
    for(NodeFinderElementClass &c : elementClasses)
    {
        if(c.getTagName() == e.tagName())
        {
            c.renameElement(e, newId, this);
            return;
        }
    }
    qWarning() << "Renaming unregistered element";
}

void NodeFinderSVGConverter::storeElement(QDomElement e)
{
    for(NodeFinderElementClass &c : elementClasses)
    {
        if(c.preocessElement(e, this))
            break; //Registered
    }
}

void NodeFinderSVGConverter::removeElement(QDomElement e, bool *isFakeId)
{
    for(NodeFinderElementClass &c : elementClasses)
    {
        if(c.getTagName() == e.tagName())
        {
            c.removeElement(e);

            const QString oldId = e.attribute(ssplib::svg_attr::ID);
            if(fakeIds.contains(oldId))
            {
                if(isFakeId)
                    *isFakeId = true;
            }

            return;
        }
    }
}

int NodeFinderSVGConverter::getCurItemSubElemIdx() const
{
    return curItemSubElemIdx;
}

void NodeFinderSVGConverter::setCurItemSubElemIdx(int value)
{
    curItemSubElemIdx = value;
}

ssplib::ItemBase *NodeFinderSVGConverter::getCurItem() const
{
    return curItem;
}

void NodeFinderSVGConverter::setCurItem(ssplib::ItemBase *value)
{
    curItem = value;
    curItemSubElemIdx = -1;
}
