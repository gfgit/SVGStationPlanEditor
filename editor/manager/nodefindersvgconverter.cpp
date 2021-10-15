#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

#include "model/nodefinderlabelmodel.h"
#include "model/nodefinderstationtracksmodel.h"
#include "model/nodefinderturnoutmodel.h"

#include "ssplib/utils/svg_path_utils.h"
#include "ssplib/utils/svg_constants.h"

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

    labelsModel->clear();
    tracksModel->clear();
    turnoutModel->clear();

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

void NodeFinderSVGConverter::loadLabelsAndTracks()
{
    QVector<ssplib::LabelItem> labels;
    QVector<ssplib::TrackItem> tracks;
    QVector<ssplib::TrackConnectionItem> connections;

    QStringList tags{ssplib::svg_tags::RectTag,
                     ssplib::svg_tags::PathTag,
                     ssplib::svg_tags::LineTag,
                     ssplib::svg_tags::PolylineTag};

    auto walker = walkElements(tags);

    while (walker.next())
    {
        QDomElement e = walker.element();

        parseLabel(e, labels);

        parsePlatform(e, tracks);

        parseTrackConnection(e, connections);
    }

    //Sort models
    std::sort(labels.begin(), labels.end());
    std::sort(tracks.begin(), tracks.end());
    std::sort(connections.begin(), connections.end());

    labelsModel->setItems(labels);
    tracksModel->setItems(tracks);
    turnoutModel->setItems(connections);
}

void NodeFinderSVGConverter::processElements()
{
    QDomElement root = mDoc.documentElement();

    int generatedIdSerial = 0;
    const QString generatedIdBase = QLatin1String("generated_");

    //Treat root as a group
    processGroup(root, generatedIdSerial, generatedIdBase);
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

void NodeFinderSVGConverter::processDefs(QDomElement &g)
{
    QDomNode n = g.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull())
        {
            if(e.tagName() == ssplib::svg_tags::DefsTag)
            {
                processDefs(e);
            }
            else if(e.tagName() == ssplib::svg_tags::FontTag)
            {
                //Add ID if missing
                if(!e.hasAttribute(ssplib::svg_attr::ID))
                {
                    int counter = 0;
                    const QString newId = getFreeId_internal(QLatin1String("font_"), counter);
                    if(newId.isEmpty())
                        qWarning() << "EMPTY FONT ID";
                    else
                        e.setAttribute(ssplib::svg_attr::ID, newId);
                }
            }
        }
        n = n.nextSibling();
    }
}

void NodeFinderSVGConverter::processGroup(QDomElement &g, int &generatedIdSerial, const QString& generatedIdBase)
{
    QDomNode n = g.firstChild();
    while(!n.isNull())
    {
        // Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(!e.isNull())
        {
            // The node really is an element.

            if(e.hasAttribute(ssplib::svg_attr::ID))
            {
                QString id = e.attribute(ssplib::svg_attr::ID);
                if(namedElements.contains(id))
                {
                    //Duplicate id, rename element
                    id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                    if(id.isEmpty())
                        e.removeAttribute(ssplib::svg_attr::ID);
                    else
                        e.setAttribute(ssplib::svg_attr::ID, id);
                }
                namedElements.insert(id, e);
            }

            storeElement(e);

            if(e.tagName() == ssplib::svg_tags::GroupTag)
            {
                //Process also sub elements
                processGroup(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == ssplib::svg_tags::TextTag)
            {
                processText(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == ssplib::svg_tags::DefsTag)
            {
                processDefs(e);
            }
        }
        n = n.nextSibling();
    }
}

void NodeFinderSVGConverter::processText(QDomElement &text, int &generatedIdSerial, const QString& generatedIdBase)
{
    //xml:space="preserve" moves text to right, remove it
    text.removeAttribute(ssplib::svg_attr::XmlSpace);

    QDomNode n = text.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Skip normal text (leave it as is)
            n = n.nextSibling();
            continue;
        }

        //Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(e.isNull())
        {
            //Node is not an element, skip it
            n = n.nextSibling();
            continue;
        }

        //The node really is an element.
        if(e.tagName() == ssplib::svg_tags::TextTag)
        {
            //Remove text elements inside a text element
            qDebug() << "TEXT inside TEXT" << e.lineNumber() << e.columnNumber();
            QDomNode old = n;
            n = n.nextSibling();
            text.removeChild(old);
        }
        else
        {
            if(e.tagName() == ssplib::svg_tags::TSpanTag)
            {
                if(e.hasAttribute(ssplib::svg_attr::ID))
                {
                    QString id = e.attribute(ssplib::svg_attr::ID);
                    if(namedElements.contains(id))
                    {
                        //Duplicate id, rename element
                        id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                        if(id.isEmpty())
                            e.removeAttribute(ssplib::svg_attr::ID);
                        else
                            e.setAttribute(ssplib::svg_attr::ID, id);
                    }
                    namedElements.insert(id, e);
                }

                processTspan(e, text);
            }

            n = n.nextSibling();
        }
    }
}

void NodeFinderSVGConverter::processTspan(QDomElement &tspan, QDomElement &text)
{
    QString value;

    QDomNode n = tspan.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Keep text
            value.append(n.nodeValue());
            QDomNode old = n;
            n = n.nextSibling();
            tspan.removeChild(old);
            continue;
        }

        // Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(!e.isNull())
        {
            // the node really is an element.
            if(e.tagName() == ssplib::svg_tags::TSpanTag)
            {
                processInternalTspan(tspan, e, value);
            }
        }
        n = n.nextSibling();
        if(!e.isNull())
        {
            //Remove internal tspan
            tspan.removeChild(e);
        }
    }

    for(const QString& attr : ssplib::svg_attr::TSpanPassToTextAttrs)
    {
        if(tspan.hasAttribute(attr))
        {
            text.setAttribute(attr, tspan.attribute(attr));
            tspan.removeAttribute(attr);
        }
    }

    QDomText textVal = mDoc.createTextNode(value);
    tspan.appendChild(textVal);
}

void NodeFinderSVGConverter::processInternalTspan(QDomElement &top, QDomElement &cur, QString &value)
{
    QDomNode n = cur.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Keep text
            value.append(n.nodeValue());
            n = n.nextSibling();
            continue;
        }

        // Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(!e.isNull())
        {
            // the node really is an element.
            if(e.tagName() == ssplib::svg_tags::TSpanTag)
            {
                processInternalTspan(top, e, value);
            }
        }

        n = n.nextSibling();
    }

    for(const QString& attr : ssplib::svg_attr::TSpanPassAttrs)
    {
        if(cur.hasAttribute(attr))
            top.setAttribute(attr, cur.attribute(attr));
    }
}

bool NodeFinderSVGConverter::parseLabel(QDomElement &e, QVector<ssplib::LabelItem> &labels)
{
    QString labelName = e.attribute(ssplib::svg_attr::LabelName);
    if(labelName.isEmpty())
        return false;

    bool ok = true;
    labelName = labelName.simplified();

    if(labelName.isEmpty() || labelName.front() < 'A' || labelName.front() > 'Z')
    {
        ok = false;
    }

    ssplib::ElementPath elemPath;
    elemPath.elem = e;

    if(ok)
    {
        ok = ssplib::utils::convertElementToPath(elemPath.elem, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(ssplib::svg_attr::LabelName);
        return false;
    }

    QChar gateLetter = labelName.front();

    int i = 0;
    for(; i < labels.size(); i++)
    {
        if(labels.at(i).gateLetter == gateLetter)
            break; //Label already exists, add the new element
    }
    if(i >= labels.size())
    {
        //Create new label
        ssplib::LabelItem newItem;
        newItem.gateLetter = gateLetter;
        newItem.visible = false;
        labels.append(newItem);
        i = labels.size() - 1;
    }

    elemPath.strokeWidth = 0;
    const QRectF bounds = elemPath.path.boundingRect();
    if(!ssplib::utils::parseStrokeWidth(elemPath.elem, bounds, elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    //Add element to label
    ssplib::LabelItem &item = labels[i];
    item.elements.append(elemPath);

    return true;
}

bool NodeFinderSVGConverter::parsePlatform(QDomElement &e, QVector<ssplib::TrackItem> &platforms)
{
    QString trackPosStr = e.attribute(ssplib::svg_attr::TrackPos);
    if(trackPosStr.isEmpty())
        return false;

    bool ok = false;
    int trackPos = trackPosStr.toInt(&ok);

    ssplib::ElementPath elemPath;
    elemPath.elem = e;

    if(ok)
    {
        ok = ssplib::utils::convertElementToPath(elemPath.elem, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(ssplib::svg_attr::TrackPos);
        return false;
    }

    int i = 0;
    for(; i < platforms.size(); i++)
    {
        if(platforms.at(i).trackPos == trackPos)
            break; //Platform exists
    }
    if(i >= platforms.size())
    {
        //Create new platform
        ssplib::TrackItem newItem;
        //newItem.trackName = ...; //TODO: real name from database
        newItem.trackPos = trackPos;
        newItem.visible = false;
        platforms.append(newItem);
        i = platforms.size() - 1;
    }

    elemPath.strokeWidth = 0;
    const QRectF bounds = elemPath.path.boundingRect();
    if(!ssplib::utils::parseStrokeWidth(elemPath.elem, bounds, elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    //Add element to platform
    ssplib::TrackItem &item = platforms[i];
    item.elements.append(elemPath);

    return true;
}

bool NodeFinderSVGConverter::parseTrackConnection(QDomElement &e, QVector<ssplib::TrackConnectionItem> &connections)
{
    QString trackConnStr = e.attribute(ssplib::svg_attr::TrackConnections);
    if(trackConnStr.isEmpty())
        return false;

    QVector<ssplib::TrackConnectionInfo> infoVec;
    bool ok = ssplib::utils::parseTrackConnectionAttribute(trackConnStr, infoVec);

    ssplib::ElementPath elemPath;
    elemPath.elem = e;

    if(ok)
    {
        ok = ssplib::utils::convertElementToPath(elemPath.elem, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(ssplib::svg_attr::TrackConnections);
        return false;
    }

    elemPath.strokeWidth = 0;
    const QRectF bounds = elemPath.path.boundingRect();
    if(!ssplib::utils::parseStrokeWidth(elemPath.elem, bounds, elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    for(const ssplib::TrackConnectionInfo& info : qAsConst(infoVec))
    {
        //Find track connection
        int i = 0;
        for(; i < connections.size(); i++)
        {
            if(connections.at(i).info == info)
                break; //Connection exists
        }
        if(i >= connections.size())
        {
            //Create new platform
            ssplib::TrackConnectionItem newItem;
            newItem.info = info;
            newItem.visible = false;
            connections.append(newItem);
            i = connections.size() - 1;
        }

        ssplib::TrackConnectionItem &item = connections[i];
        item.elements.append(elemPath);
    }

    return true;
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
