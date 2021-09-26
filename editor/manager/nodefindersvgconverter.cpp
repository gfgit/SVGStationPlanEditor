#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

#include "editor/model/nodefinderlabelmodel.h"
#include "editor/model/nodefinderstationtracksmodel.h"

#include "utils/svgutils.h"

#include <QSvgRenderer>

#include <QTemporaryFile>

#include <QDebug>

NodeFinderSVGConverter::NodeFinderSVGConverter(NodeFinderMgr *parent) :
    QObject(parent),
    nodeMgr(parent),
    curItem(nullptr)
{
    registerClass(svg_tag::GroupTag); //Groups
    registerClass(svg_tag::RectTag);
    registerClass(svg_tag::PathTag);
    registerClass(svg_tag::LineTag);
    registerClass(svg_tag::PolylineTag);

    mSvg = new QSvgRenderer(this);

    labelsModel = new NodeFinderLabelModel(nodeMgr, this);
    tracksModel = new NodeFinderStationTracksModel(nodeMgr, this);
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

    currentWalker = NodeFinderElementWalker();
    curItem = nullptr;
    curItemSubElemIdx = -1;
    curElementPath = ElementPath();

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
    QVector<LabelItem> labels;
    QVector<TrackItem> tracks;
    QVector<TrackConnectionItem> connections;

    QStringList tags{svg_tag::RectTag, svg_tag::PathTag, svg_tag::LineTag, svg_tag::PolylineTag};
    auto walker = walkElements(tags);

    while (walker.next())
    {
        QDomElement e = walker.element();

        parseLabel(e, labels);

        parsePlatform(e, tracks);

        parseTrackConnection(e, connections);
    }

    labelsModel->setItems(labels);
    tracksModel->setItems(tracks);
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
        e.removeAttribute(svg_attr::ID);
    }
}

void NodeFinderSVGConverter::restoreFakeIDs()
{
    //Restore generated IDs
    for(auto it = fakeIds.begin(); it != fakeIds.end(); it++)
    {
        it.value().setAttribute(svg_attr::ID, it.key());
    }
}

QAbstractItemModel *NodeFinderSVGConverter::getLabelsModel() const
{
    return labelsModel;
}

QAbstractItemModel *NodeFinderSVGConverter::getTracksModel() const
{
    return tracksModel;
}

void NodeFinderSVGConverter::removeCurrentSubElementFromItem()
{
    if(!curItem || curItemSubElemIdx < 0 || curItemSubElemIdx >= curItem->elements.size())
        return;

    ElementPath path = curItem->elements.takeAt(curItemSubElemIdx);
    curItemSubElemIdx = -1; //Reset sub element selection

    //TODO: properly update models
    if(nodeMgr->mode() == EditingModes::LabelEditing)
    {
        path.elem.removeAttribute(svg_attr::LabelName);
        QModelIndex idx;
        emit labelsModel->dataChanged(idx, idx);
    }
    else if(nodeMgr->mode() == EditingModes::StationTrackEditing)
    {
        tracksModel->clearElement(path);
        QModelIndex idx;
        emit tracksModel->dataChanged(idx, idx);
    }
}

bool NodeFinderSVGConverter::addCurrentElementToItem()
{
    if(!currentWalker.isValid() || !curItem)
        return false;

    ElementPath path;
    path.elem = currentWalker.element();
    if(!utils::convertElementToPath(path.elem, path.path))
        return false;

    //TODO: properly update models
    if(nodeMgr->mode() == EditingModes::LabelEditing)
    {
        QChar gateLetter = labelsModel->getLabelLetter(curItem);
        if(gateLetter.isNull())
            return false;

        path.elem.setAttribute(svg_attr::LabelName, gateLetter);
        curItem->elements.append(path);

        QModelIndex idx;
        emit labelsModel->dataChanged(idx, idx);
    }
    else if(nodeMgr->mode() == EditingModes::StationTrackEditing)
    {
        int trackPos = tracksModel->getTrackPos(curItem);
        if(trackPos < 0)
            return false;

        path.elem.setAttribute(svg_attr::TrackPos, QString::number(trackPos));
        curItem->elements.append(path);

        QModelIndex idx;
        emit tracksModel->dataChanged(idx, idx);
    }

    return true;
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

void NodeFinderSVGConverter::processDefs(QDomElement &g)
{
    QDomNode n = g.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull())
        {
            if(e.tagName() == svg_tag::DefsTag)
            {
                processDefs(e);
            }
            else if(e.tagName() == svg_tag::FontTag)
            {
                //Add ID if missing
                if(!e.hasAttribute(svg_attr::ID))
                {
                    int counter = 0;
                    const QString newId = getFreeId_internal(QLatin1String("font_"), counter);
                    if(newId.isEmpty())
                        qWarning() << "EMPTY FONT ID";
                    else
                        e.setAttribute(svg_attr::ID, newId);
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

            if(e.hasAttribute(svg_attr::ID))
            {
                QString id = e.attribute(svg_attr::ID);
                if(namedElements.contains(id))
                {
                    //Duplicate id, rename element
                    id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                    if(id.isEmpty())
                        e.removeAttribute(svg_attr::ID);
                    else
                        e.setAttribute(svg_attr::ID, id);
                }
                namedElements.insert(id, e);
            }

            storeElement(e);

            if(e.tagName() == svg_tag::GroupTag)
            {
                //Process also sub elements
                processGroup(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == svg_tag::TextTag)
            {
                processText(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == svg_tag::DefsTag)
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
    text.removeAttribute(svg_attr::XmlSpace);

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
        if(e.tagName() == svg_tag::TextTag)
        {
            //Remove text elements inside a text element
            qDebug() << "TEXT inside TEXT" << e.lineNumber() << e.columnNumber();
            QDomNode old = n;
            n = n.nextSibling();
            text.removeChild(old);
        }
        else
        {
            if(e.tagName() == svg_tag::TSpanTag)
            {
                if(e.hasAttribute(svg_attr::ID))
                {
                    QString id = e.attribute(svg_attr::ID);
                    if(namedElements.contains(id))
                    {
                        //Duplicate id, rename element
                        id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                        if(id.isEmpty())
                            e.removeAttribute(svg_attr::ID);
                        else
                            e.setAttribute(svg_attr::ID, id);
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
            if(e.tagName() == svg_tag::TSpanTag)
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

    for(const QString& attr : svg_attr::TSpanPassToTextAttrs)
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
            if(e.tagName() == svg_tag::TSpanTag)
            {
                processInternalTspan(top, e, value);
            }
        }

        n = n.nextSibling();
    }

    for(const QString& attr : svg_attr::TSpanPassAttrs)
    {
        if(cur.hasAttribute(attr))
            top.setAttribute(attr, cur.attribute(attr));
    }
}

bool NodeFinderSVGConverter::parseLabel(QDomElement &e, QVector<LabelItem> &labels)
{
    QString labelName = e.attribute(svg_attr::LabelName);
    if(labelName.isEmpty())
        return false;

    labelName = labelName.simplified();
    if(labelName.isEmpty() || labelName.front() < 'A' || labelName.front() > 'Z')
    {
        e.removeAttribute(svg_attr::LabelName);
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
        LabelItem newItem;
        newItem.gateLetter = gateLetter;
        newItem.visible = false;
        labels.append(newItem);
        i = labels.size() - 1;
    }

    //Add element to label
    QPainterPath path;
    path.addRect(mSvg->boundsOnElement(e.attribute(svg_attr::ID)));

    LabelItem &item = labels[i];
    item.elements.append({e, path});

    return true;
}

bool NodeFinderSVGConverter::parsePlatform(QDomElement &e, QVector<TrackItem> &platforms)
{
    QString trackPosStr = e.attribute(svg_attr::TrackPos);
    if(trackPosStr.isEmpty())
        return false;

    bool ok = false;
    int trackPos = trackPosStr.toInt(&ok);

    QPainterPath path;
    if(ok)
    {
        ok = utils::convertElementToPath(e, path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(svg_attr::TrackPos);
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
        TrackItem newItem;
        newItem.trackName = QString::number(trackPos); //TODO: real name from database
        newItem.trackPos = trackPos;
        newItem.visible = false;
        platforms.append(newItem);
        i = platforms.size() - 1;
    }

    TrackItem &item = platforms[i];
    item.elements.append({e, path});

    return true;
}

bool NodeFinderSVGConverter::parseTrackConnection(QDomElement &e, QVector<TrackConnectionItem> &connections)
{
    QString trackConnStr = e.attribute(svg_attr::TrackConnections);
    if(trackConnStr.isEmpty())
        return false;

    QVector<TrackConnectionInfo> infoVec;
    bool ok = utils::parseTrackConnectionAttribute(trackConnStr, infoVec);

    QPainterPath path;
    if(ok)
    {
        ok = utils::convertElementToPath(e, path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(svg_attr::TrackConnections);
        return false;
    }

    for(const TrackConnectionInfo& info : qAsConst(infoVec))
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
            TrackConnectionItem newItem;
            newItem.info = info;
            newItem.visible = false;
            connections.append(newItem);
            i = connections.size() - 1;
        }

        TrackConnectionItem &item = connections[i];
        item.elements.append({e, path});
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

ItemBase *NodeFinderSVGConverter::getCurItem() const
{
    return curItem;
}

void NodeFinderSVGConverter::setCurItem(ItemBase *value)
{
    curItem = value;
    curItemSubElemIdx = -1;
}
