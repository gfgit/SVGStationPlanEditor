#include "xmlelement.h"

using namespace ssplib::utils;

XmlElement::XmlElement() :
    isDom(false)
{

}

XmlElement::XmlElement(const QString &tag, const QXmlStreamAttributes &attrs) :
    streamElem{attrs, tag}, isDom(false)
{

}

XmlElement::XmlElement(const QStringRef &tag, const QXmlStreamAttributes &attrs) :
    XmlElement(tag.toString(), attrs)
{

}

XmlElement::XmlElement(QDomElement &e) :
    domElem(e), isDom(true)
{

}

bool XmlElement::hasAttribute(const QString &name) const
{
#ifdef SSPLIB_ENABLE_EDITING
    if(isDom)
    {
        return domElem.hasAttribute(name);
    }
#endif

    return streamElem.attrs.hasAttribute(name);
}

QString XmlElement::attribute(const QString &name) const
{
#ifdef SSPLIB_ENABLE_EDITING
    if(isDom)
    {
        return domElem.attribute(name);
    }
#endif

    return streamElem.attrs.value(name).toString();
}

QString XmlElement::tagName() const
{
#ifdef SSPLIB_ENABLE_EDITING
    if(isDom)
    {
        return domElem.tagName();
    }
#endif

    return streamElem.tag;
}

void XmlElement::removeAttribute(const QString &name)
{
#ifdef SSPLIB_ENABLE_EDITING
    if(isDom)
    {
        domElem.removeAttribute(name);
    }
#endif
}
