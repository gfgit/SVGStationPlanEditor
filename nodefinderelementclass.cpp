#include "nodefinderelementclass.h"

#include "nodefindersvgconverter.h"

const QString NodeFinderElementClass::idAttr = QLatin1String("id");

NodeFinderElementClass::NodeFinderElementClass(const QString &tag, const QString &baseId) :
    tagName(tag),
    m_baseId(baseId),
    serial(0)
{

}

QString NodeFinderElementClass::getTagName() const
{
    return tagName;
}

bool NodeFinderElementClass::preocessElement(QDomElement e, NodeFinderSVGConverter *conv)
{
    if(e.tagName() != tagName)
        return false;

    QString id = e.attribute(idAttr);
    if(id.isEmpty())
    {
        //Generate a fake ID so we can query
        //element's bounds from Qt SVG renderer.
        //When saving this ID will be removed.

        id = conv->getFreeId_internal(m_baseId, serial);

        if(!id.isEmpty())
        {
            //Remember the generated ID to remove it later
            e.setAttribute(idAttr, id);
            conv->fakeIds.insert(id, e);
        }
    }

    if(!id.isEmpty())
    {
        hash.insert(id, e);
    }

    return true;
}

void NodeFinderElementClass::clear()
{
    serial = 0;
    hash.clear();
    hash.squeeze();
}

NodeFinderElementClass::CallbackResult NodeFinderElementClass::walkElements(QDomElement &result, Callback fun)
{
    for(QDomElement& e : hash)
    {
        CallbackResult res = fun(e);

        if(res == KeepSearching)
            continue;

        if(res == CallbackResult::ReturnCurrentElement)
        {
            result = e;
        }
        return res;
    }
    return KeepSearching;
}

void NodeFinderElementClass::renameElement(QDomElement &e, const QString& newId, NodeFinderSVGConverter *conv)
{
    const QString oldId = e.attribute(idAttr);

    //Remove from fake and do not insert back, because now it is used
    conv->fakeIds.remove(oldId);
    conv->namedElements.remove(oldId);
    hash.remove(oldId);

    if(newId.isEmpty())
    {
        //No new ID
        e.removeAttribute(idAttr);
    }
    else
    {
        e.setAttribute(idAttr, newId);
        hash.insert(newId, e);
        conv->namedElements.insert(newId, e);
    }
}
