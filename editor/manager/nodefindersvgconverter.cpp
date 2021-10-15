#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

#include "model/nodefinderlabelmodel.h"
#include "model/nodefinderstationtracksmodel.h"
#include "model/nodefinderturnoutmodel.h"

#include "ssplib/utils/svg_path_utils.h"
#include "ssplib/utils/svg_constants.h"

#include "ssplib/parsing/domparser.h"

#include <QSvgRenderer>

#include <QTemporaryFile>

#include <QDebug>

NodeFinderSVGConverter::NodeFinderSVGConverter(NodeFinderMgr *parent) :
    QObject(parent),
    nodeMgr(parent),
    curItem(nullptr)
{
    registerClass(ssplib::svg_tags::GroupTag); //Groups
    registerClass(ssplib::svg_tags::RectTag);
    registerClass(ssplib::svg_tags::PathTag);
    registerClass(ssplib::svg_tags::LineTag);
    registerClass(ssplib::svg_tags::PolylineTag);

    mSvg = new QSvgRenderer(this);

    labelsModel = new NodeFinderLabelModel(nodeMgr, this);
    tracksModel = new NodeFinderStationTracksModel(nodeMgr, this);
    turnoutModel = new NodeFinderTurnoutModel(nodeMgr, this);
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

    namedElements.clear();
    namedElements.squeeze();

    m_plan.clear();
    m_info.clear();

    currentWalker = NodeFinderElementWalker();
    curItem = nullptr;
    curItemSubElemIdx = -1;
    curElementPath = ssplib::ElementPath();

    mDoc.clear();
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
    const int IndentSize = 3;

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
    if(trackPenWidth < 10)
        trackPenWidth = 10;
    return trackPenWidth;
}

void NodeFinderSVGConverter::processElements()
{
    ssplib::DOMParser parser(&mDoc, &m_plan, &m_info);
    parser.parse();
}

QDomElement NodeFinderSVGConverter::elementById(const QString &id)
{
    auto it = namedElements.constFind(id);
    if(it != namedElements.constEnd())
        return it.value();

    it = fakeIds.constFind(id);
    if(it != fakeIds.constEnd())
        return it.value();

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
    if(!ssplib::utils::parseStrokeWidth(path.elem, bounds, path.strokeWidth))
        path.strokeWidth = 0;

    return model->addElementToItem(path, curItem);
}

QString NodeFinderSVGConverter::getFreeId_internal(const QString &base, int &counter)
{
    QString fmt = base + QLatin1String("%1");
    for(int i = 0; i < 2000; i++)
    {
        const QString id = fmt.arg(counter++);
        if(elementById(id).isNull())
            return id;
    }
    return QString();
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
