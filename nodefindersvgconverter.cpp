#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

#include "nodefinderlabelmodel.h"
#include "nodefinderstationtracksmodel.h"

#include "svgutils.h"

#include <QSvgRenderer>

#include <QTemporaryFile>

#include <QDebug>

static const QStringList tspanPassAttrs{"x", "y", "fill", "stroke", "font-family", "font-size", "font-weight"};
static const QStringList tspanPassToTextAttrs{"x", "y"};

static const QString defsTag = QLatin1String("defs");
static const QString fontTag = QLatin1String("font");
static const QString textTag = QLatin1String("text");
static const QString tspanTag = QLatin1String("tspan");

NodeFinderSVGConverter::NodeFinderSVGConverter(NodeFinderMgr *parent) :
    QObject(parent),
    nodeMgr(parent)
{
    registerClass("g"); //Groups
    registerClass("rect");
    registerClass("path");
    registerClass("line");
    registerClass("polyline");

    mSvg = new QSvgRenderer(this);

    labelsModel = new NodeFinderLabelModel(this);
    tracksModel = new NodeFinderStationTracksModel(this);
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

    mDoc.clear();
}

bool NodeFinderSVGConverter::load(QIODevice *dev)
{
    //FIXME: error reporting
    clear();

    QXmlStreamReader xml(dev);
    xml.setNamespaceProcessing(false);

    bool ret = mDoc.setContent(&xml, xml.namespaceProcessing());
    if(!ret)
        return false;

    //Read again from start
    ret = dev->reset();
    xml.setDevice(dev);
    if(!ret)
        return false;

    ret = mSvg->load(&xml);

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

void NodeFinderSVGConverter::loadLabelsAndTracks()
{
    QVector<NodeFinderLabelModel::LabelItem> labels;
    QVector<NodeFinderStationTracksModel::TrackItem> tracks;

    const QString labelAttr = QLatin1String("labelname");
    const QString trackAttr = QLatin1String("trackpos");

    auto fun = [&](QDomElement& e) -> NodeFinderElementClass::CallbackResult
    {
        QString labelName = e.attribute(labelAttr);
        if(!labelName.isEmpty())
        {
            //FIXME: allow only A-Z
            labelName = labelName.simplified();
            if(labelName.isEmpty())
            {
                e.removeAttribute(labelAttr);
            }
            else
            {
                NodeFinderLabelModel::LabelItem item;
                item.gateLetter = labelName.front();
                item.elem = e;
                item.rect = mSvg->boundsOnElement(e.attribute(NodeFinderElementClass::idAttr));
                item.visible = true;
                labels.append(item);
            }

            return NodeFinderElementClass::CallbackResult::KeepSearching;
        }

        QString trackPosStr = e.attribute(labelAttr);
        if(!trackPosStr.isEmpty())
        {
            bool ok = false;
            int trackPos = trackPosStr.toInt(&ok);

            QPainterPath path;
            if(ok)
            {
                ok = utils::convertElementToPath(e, path);
            }

            if(!ok)
            {
                e.removeAttribute(trackAttr);
            }
            else
            {
                NodeFinderStationTracksModel::TrackItem item;
                item.trackName = QString::number(trackPos); //TODO: real name from database
                item.elem = e;
                item.path = path;
                item.visible = false;
                tracks.append(item);
            }

            return NodeFinderElementClass::CallbackResult::KeepSearching;
        }

        return NodeFinderElementClass::CallbackResult::KeepSearching;
    };


    QDomElement unused;
    auto rects = elementClasses.value(QLatin1String("rect"));
    rects.walkElements(unused, fun);
    auto paths = elementClasses.value(QLatin1String("path"));
    paths.walkElements(unused, fun);
    auto lines = elementClasses.value(QLatin1String("line"));
    lines.walkElements(unused, fun);
    auto polylines = elementClasses.value(QLatin1String("polyline"));
    polylines.walkElements(unused, fun);

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
        e.removeAttribute(NodeFinderElementClass::idAttr);
    }
}

void NodeFinderSVGConverter::restoreFakeIDs()
{
    //Restore generated IDs
    for(auto it = fakeIds.begin(); it != fakeIds.end(); it++)
    {
        it.value().setAttribute(NodeFinderElementClass::idAttr, it.key());
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
            if(e.tagName() == defsTag)
            {
                processDefs(e);
            }
            else if(e.tagName() == fontTag)
            {
                //Add ID if missing
                if(!e.hasAttribute(NodeFinderElementClass::idAttr))
                {
                    int counter = 0;
                    const QString newId = getFreeId_internal(QLatin1String("font_"), counter);
                    if(newId.isEmpty())
                        qWarning() << "EMPTY FONT ID";
                    else
                        e.setAttribute(NodeFinderElementClass::idAttr, newId);
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

            if(e.hasAttribute(NodeFinderElementClass::idAttr))
            {
                QString id = e.attribute(NodeFinderElementClass::idAttr);
                if(namedElements.contains(id))
                {
                    //Duplicate id, rename element
                    id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                    if(id.isEmpty())
                        e.removeAttribute(NodeFinderElementClass::idAttr);
                    else
                        e.setAttribute(NodeFinderElementClass::idAttr, id);
                }
                namedElements.insert(id, e);
            }

            storeElement(e);

            if(e.tagName() == 'g')
            {
                //Process also sub elements
                processGroup(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == textTag)
            {
                processText(e, generatedIdSerial, generatedIdBase);
            }
            else if(e.tagName() == defsTag)
            {
                processDefs(e);
            }
        }
        n = n.nextSibling();
    }
}

void NodeFinderSVGConverter::processText(QDomElement &text, int &generatedIdSerial, const QString& generatedIdBase)
{
    QDomNode n = text.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Skip normal text
            n = n.nextSibling();
        }
        else
        {
            QDomElement e = n.toElement(); //Try to convert the node to an element.
            if(!e.isNull())
            {
                //The node really is an element.
                if(e.tagName() == textTag)
                {
                    qDebug() << "TEXT inside TEXT" << e.lineNumber() << e.columnNumber();
                    QDomNode old = n;
                    text.removeChild(old);
                    n = n.nextSibling();
                }
                else
                {
                    if(e.tagName() == tspanTag)
                    {
                        if(e.hasAttribute(NodeFinderElementClass::idAttr))
                        {
                            QString id = e.attribute(NodeFinderElementClass::idAttr);
                            if(namedElements.contains(id))
                            {
                                //Duplicate id, rename element
                                id = getFreeId_internal(generatedIdBase, generatedIdSerial);
                                if(id.isEmpty())
                                    e.removeAttribute(NodeFinderElementClass::idAttr);
                                else
                                    e.setAttribute(NodeFinderElementClass::idAttr, id);
                            }
                            namedElements.insert(id, e);
                        }

                        processTspan(e, text);
                    }

                    n = n.nextSibling();
                }
            }else{
                n = n.nextSibling();
            }
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
            value.append(n.nodeValue());
            QDomNode old = n;
            n = n.nextSibling();
            tspan.removeChild(old);
        }
        else
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull())
            {
                // the node really is an element.
                if(e.tagName() == QLatin1String("tspan"))
                {
                    processInternalTspan(tspan, e, value);
                }
            }
            n = n.nextSibling();
            if(!e.isNull())
            {
                tspan.removeChild(e);
            }
        }

    }

    for(const QString& attr : tspanPassToTextAttrs)
    {
        if(!text.hasAttribute(attr))
            text.setAttribute(attr, tspan.attribute(attr));
        tspan.removeAttribute(attr);
    }

    QDomText textVal = mDoc.createTextNode(value);
    tspan.appendChild(textVal);
}

void NodeFinderSVGConverter::processInternalTspan(QDomElement &top, QDomElement &cur, QString &value)
{
    for(const QString& attr : tspanPassAttrs)
    {
        if(!top.hasAttribute(attr) && cur.hasAttribute(attr))
            top.setAttribute(attr, cur.attribute(attr));
    }

    QDomNode n = cur.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            value.append(n.nodeValue());
        }
        else
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull())
            {
                // the node really is an element.
                if(e.tagName() == QLatin1String("tspan"))
                {
                    processInternalTspan(top, e, value);
                }
            }
        }

        n = n.nextSibling();
    }
}
