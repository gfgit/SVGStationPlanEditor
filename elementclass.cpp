#include "elementclass.h"

#include "svgtinyconverter.h"

ElementClass::ElementClass(SVGTinyConverter *c, const QString &tag, const QString &baseId) :
    conv(c),
    tagName(tag),
    m_baseId(baseId),
    serial(0)
{

}

QString ElementClass::getTagName() const
{
    return tagName;
}

bool ElementClass::preocessElement(QDomElement e)
{
    if(e.tagName() != tagName)
        return false;

    QString id = e.attribute(SVGTinyConverter::idAttr);
    if(id.isEmpty())
    {
        //Generate a fake ID so we can query
        //element's bounds from Qt SVG renderer.
        //When saving this ID will be removed.

        id = conv->getFreeId_internal(m_baseId, serial);

        if(!id.isEmpty())
        {
            //Remember the generated ID to remove later
            e.setAttribute(SVGTinyConverter::idAttr, id);
            conv->fakeIds.insert(id, e);
        }
    }

    if(!id.isEmpty())
    {
        hash.insert(id, e);
    }

    return true;
}

void ElementClass::clear()
{
    serial = 0;
    hash.clear();
    hash.squeeze();
}

ElementClass::CallbackResult ElementClass::walkElements(QDomElement &result, Callback fun) const
{
    for(const QDomElement& e : qAsConst(hash))
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

void ElementClass::renameElement(QDomElement &e, const QString& newId)
{
    const QString oldId = e.attribute(SVGTinyConverter::idAttr);

    //Remove from fake and do not insert back, because now it is used
    conv->fakeIds.remove(oldId);
    hash.remove(oldId);

    if(newId.isEmpty())
    {
        //No new ID
        e.removeAttribute(SVGTinyConverter::idAttr);
    }
    else
    {
        e.setAttribute(SVGTinyConverter::idAttr, newId);
        hash.insert(newId, e);
    }
}
