#include "domparser.h"

#include <ssplib/utils/svg_constants.h>

#include <ssplib/stationplan.h>

#include "editinginfo.h"

#include "parsinghelpers.h"
#include <ssplib/utils/transform_utils.h>

#include <QDebug>

#ifdef SSPLIB_ENABLE_EDITING

using namespace ssplib;

void applyTransform(utils::Transform& transf, QDomElement& e)
{
    QString val = transf.value;
    QString newVal = e.attribute(svg_attr::Transform).trimmed();
    if(!val.isEmpty() && !newVal.isEmpty())
    {
        //Separate from parent transform
        val.append('\n');
    }
    val.append(newVal);
    if(!val.isEmpty())
        e.setAttribute(svg_attr::Transform, val);
}

DOMParser::DOMParser(QDomDocument *doc, StationPlan *ptr, EditingInfo *info) :
    m_doc(doc),
    plan(ptr),
    m_info(info)
{

}

bool DOMParser::parse()
{
    QDomElement root = m_doc->documentElement();
    processGroup(root, utils::ElementStyle(), utils::Transform());
    return true;
}

void DOMParser::processGroup(QDomElement &g, const utils::ElementStyle& parentStyle, const utils::Transform &parentTransf)
{
    utils::ElementStyle elemStyle = utils::parseStrokeWidthStyle(g, parentStyle, QRectF());

    utils::Transform groupTransf = parentTransf;
    if(g.hasAttribute(svg_attr::Transform))
    {
        groupTransf = utils::combineTransform(parentTransf, g.attribute(svg_attr::Transform));
        g.removeAttribute(svg_attr::Transform); //Remove it once processed
    }

    QDomNode n = g.firstChild();
    while(!n.isNull())
    {
        // Try to convert the node to an element.
        QDomElement e = n.toElement();
        if(!e.isNull())
        {
            // The node really is an element.
            m_info->storeElement(e);

            bool tranformProcessed = false;

            if(e.tagName() == ssplib::svg_tags::GroupTag)
            {
                //Process also sub elements
                processGroup(e, elemStyle, groupTransf);
                tranformProcessed = true;
            }
            else if(e.tagName() == ssplib::svg_tags::TextTag)
            {
                processText(e, groupTransf);
                tranformProcessed = true;
            }
            else if(e.tagName() == ssplib::svg_tags::DefsTag)
            {
                processDefs(e);
                tranformProcessed = true;
            }
            else if(parsing::isElementSupported(e.tagName()))
            {
                utils::XmlElement e2(e);

                utils::Transform elemTransf = utils::combineTransform(groupTransf, e.attribute(svg_attr::Transform));
                tranformProcessed = elemTransf.matrix.isIdentity();

                if(!tranformProcessed)
                {
                    //Transform is not identity, we need to apply transform
                    QPainterPath path;
                    if(utils::convertElementToPath(e2, path))
                    {
                        path = elemTransf.matrix.map(path);

                        //Now rebuild path
                        QString pathD;
                        if(utils::convertPathToSVG(path, pathD))
                        {
                            //Convert to path element
                            e.setTagName(ssplib::svg_tags::PathTag);

                            QStringList attrs_to_remove{
                                "points",
                                "x", "x1", "x2",
                                "y", "y1", "y2",
                                "height", "width",
                                ssplib::svg_attr::Transform
                            };

                            for(const QString& attr : attrs_to_remove)
                                e.removeAttribute(attr);

                            e.setAttribute("d", pathD);

                            tranformProcessed = true;
                        }
                    }
                }

                if(tranformProcessed)
                {
                    parsing::parseLabel(e2, plan->labels, elemStyle);
                    parsing::parsePlatform(e2, plan->platforms, elemStyle);
                    parsing::parseTrackConnection(e2, plan->trackConnections, elemStyle);
                }
            }

            if(!tranformProcessed)
            {
                //Apply transform to single element
                applyTransform(groupTransf, e);
            }
        }
        n = n.nextSibling();
    }
}

void DOMParser::processDefs(QDomElement &defs)
{
    QDomNode n = defs.firstChild();
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
                    const QString newId = m_info->getFreeId_internal(QLatin1String("font_"), counter);
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

void DOMParser::processText(QDomElement &text, utils::Transform &parentTransf)
{
    //Qt SVG doens't seem to support <text> and <tspan> elements well
    //We remove all <tspan> and put all contents and attributes in <text> elements

    //xml:space="preserve" moves text to right, remove it
    text.removeAttribute(ssplib::svg_attr::XmlSpace);

    //Apply transform to single element
    applyTransform(parentTransf, text);

    QString value;

    QDomNode n = text.firstChild();
    while(!n.isNull())
    {
        if(n.isText())
        {
            //Keep text
            value.append(n.nodeValue());
        }
        else
        {
            //Try to convert the node to an element.
            QDomElement e = n.toElement();
            if(!e.isNull() && e.tagName() == ssplib::svg_tags::TSpanTag)
            {
                //Keep tspan attributes and contents
                processInternalTspan(text, e, value);
            }
        }

        //Remove child elements from <text>
        QDomNode old = n;
        n = n.nextSibling();
        text.removeChild(old);
    }

    //Now re-create text content
    QDomText textVal = m_doc->createTextNode(value);
    text.appendChild(textVal);
}

void DOMParser::processInternalTspan(QDomElement &top, QDomElement &cur, QString &value)
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

    for(const QString& attr : ssplib::svg_attr::TSpanPassToTextAttrs)
    {
        if(cur.hasAttribute(attr))
            top.setAttribute(attr, cur.attribute(attr));
    }
}

#endif // SSPLIB_ENABLE_EDITING
