#include "svgtinyconverter.h"

#include <QTextStream>

#include <QDebug>

const QStringList tspanPassAttrs{"x", "y", "fill", "stroke", "font-family", "font-size", "font-weight"};
const QStringList tspanPassToTextAttrs{"x", "y"};

const QString SVGTinyConverter::idAttr = QLatin1String("id");


static const QString defsTag = QLatin1String("defs");
const QString fontTag = QLatin1String("font");

SVGTinyConverter::SVGTinyConverter() :
    active(false),
    processNames(false),
    convertTspan(false)
{
    registerClass("g"); //Groups
    registerClass("rect");
    registerClass("path");
    registerClass("line");
    registerClass("polyline");
}

bool SVGTinyConverter::load(QIODevice *source, QString *errMsg, int *errLine, int *errCol)
{
    clear();
    active = mDoc.setContent(source, false, errMsg, errLine, errCol);
    return active;
}

bool SVGTinyConverter::convert()
{
    QDomElement root = mDoc.documentElement();

    //Treat root as a group
    processGroup(root);

    return true;
}

void SVGTinyConverter::processGroup(QDomElement& g)
{
    QDomNode n = g.firstChild();
    while(!n.isNull())
    {
        // Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(!e.isNull())
        {
            // The node really is an element.
            if(processNames)
                storeElement(e);

            if(e.tagName() == 'g')
            {
                //Process also sub elements
                processGroup(e);
            }
            else if(convertTspan && e.tagName() == defsTag)
            {
                processDefs(e);
            }
        }
        n = n.nextSibling();
    }
}

bool SVGTinyConverter::setIDs()
{
    if(!processNames)
        return false;

    //Set label IDs
    QString fmt = QLatin1String("label_%1");
    for(LabelEntry& entry : labels)
    {
        const QString id = fmt.arg(entry.nodeName);
        if(id == entry.elem.attribute(idAttr))
            continue; //Already set

        QDomElement old = elementById(id);
        if(!old.isNull())
        {
            //Try replace existing element's ID with *_old
            const QString newId = getFreeId(id + QLatin1String("_old"));
            renameElement(old, newId);
        }

        renameElement(entry.elem, id);
    }

    //Remove unused generated IDs
    for(QDomElement& e : fakeIds)
    {
        e.removeAttribute(idAttr);
    }

    return true;
}

bool SVGTinyConverter::write(QIODevice *output)
{
    const int IndentSize = 1;

    QTextStream out(output);
    mDoc.save(out, IndentSize);
    return true;
}

void SVGTinyConverter::clear()
{
    for(ElementClass &c : elementClasses)
        c.clear();

    fakeIds.clear();
    fakeIds.squeeze();

    labels.clear();
    labels.squeeze();

    mDoc = QDomDocument();
    active = false;
}

void SVGTinyConverter::processDefs(QDomElement& g)
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
                //NOTE: some <font> elements produced by SVG Optimizers do not have ID attribute, Qt SVG complains.
                //Add ID if missing
                if(!e.hasAttribute(idAttr))
                {
                    int counter = 0;
                    const QString newId = getFreeId_internal(QLatin1String("font_"), counter);
                    if(newId.isEmpty())
                        qWarning() << "EMPTY FONT ID";
                    else
                        e.setAttribute(idAttr, newId);
                }
            }
        }
        n = n.nextSibling();
    }
}

void SVGTinyConverter::processText(QDomElement &text)
{
    QDomNode n = text.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Do nothing
            n = n.nextSibling();
        }
        else
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull())
            {
                // the node really is an element.
                if(e.tagName() == QLatin1String("text"))
                {
                    qDebug() << "TEXT inside TEXT" << e.lineNumber() << e.columnNumber();
                    QDomNode old = n;
                    text.removeChild(old);
                    n = n.nextSibling();
                }
                else
                {
                    if(e.tagName() == QLatin1String("tspan"))
                        processTspan(e, text);
                    n = n.nextSibling();
                }
            }else{
                n = n.nextSibling();
            }
        }
    }
}

void SVGTinyConverter::processTspan(QDomElement &tspan, QDomElement& text)
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

void SVGTinyConverter::processInternalTspan(QDomElement& top, QDomElement& cur, QString& value)
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

QString SVGTinyConverter::getFreeId_internal(const QString &base, int &counter)
{
    if(elementById(base).isNull())
        return base;

    QString fmt = base + QLatin1String("%1");
    for(int i = 0; i < 1000; i++)
    {
        const QString id = fmt.arg(counter++);
        if(elementById(id).isNull())
            return id;
    }
    return QString();
}

bool SVGTinyConverter::isActive() const
{
    return active;
}

QDomElement SVGTinyConverter::elementByIdRecursive(QDomElement e, const QString &id)
{
    QDomElement child = e.firstChildElement();
    while (!child.isNull())
    {
        if(child.attribute(idAttr) == id)
        {
            return child;
        }
        else
        {
            QDomElement res = elementByIdRecursive(child, id);
            if(!res.isNull())
                return res;
        }
        child = child.nextSiblingElement();
    }

    return QDomElement();
}

QDomElement SVGTinyConverter::elementById(const QString &id)
{
    auto it = fakeIds.constFind(id);
    if(it != fakeIds.constEnd())
        return it.value();

    QDomElement e;
    for(ElementClass &c : elementClasses)
    {
        if(c.getElementById(id, e))
            return e;
    }

    //Slow: scan whole document
    QDomElement root = mDoc.documentElement();
    return elementByIdRecursive(root, id);
}

bool SVGTinyConverter::walkElements(const QStringList &classPrecedence,
                                    QDomElement &result,
                                    ElementClass::Callback fun) const
{
    for(const QString& tag : classPrecedence)
    {
        auto it = elementClasses.constFind(tag);
        if(it == elementClasses.constEnd())
            continue;

        ElementClass::CallbackResult res = it.value().walkElements(result, fun);
        if(res == ElementClass::CallbackResult::ReturnCurrentElement)
            return true;
        if(res == ElementClass::CallbackResult::AbortSearch)
            return true;
    }
    return false;
}

bool SVGTinyConverter::isElementUsedAsLabel(const QDomElement &e, QChar& letterOut) const
{
    for(const SVGTinyConverter::LabelEntry& entry : qAsConst(labels))
    {
        if(entry.elem == e)
        {
            letterOut = entry.nodeName;
            return true;
        }
    }
    return false;
}

void SVGTinyConverter::setProcessNames(bool value)
{
    processNames = value;
}

void SVGTinyConverter::setConvertTspan(bool value)
{
    convertTspan = value;
}

void SVGTinyConverter::renameElement(QDomElement &e, const QString& newId)
{
    for(ElementClass &c : elementClasses)
    {
        if(c.getTagName() == e.tagName())
        {
            c.renameElement(e, newId);
        }
    }
    qWarning() << "Renaming unregistered element";
}

void SVGTinyConverter::storeElement(QDomElement e)
{
    for(ElementClass &c : elementClasses)
    {
        if(c.preocessElement(e))
            break; //Registered
    }
}
