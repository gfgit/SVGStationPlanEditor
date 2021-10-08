#ifndef XMLELEMENT_H
#define XMLELEMENT_H

#include <QXmlStreamAttributes>

#ifdef SSPLIB_ENABLE_EDITING
#include <QDomElement>
#endif

namespace ssplib {

namespace utils {

class XmlElement
{
public:
    XmlElement();
    XmlElement(const QString& tag, const QXmlStreamAttributes& attrs);

#ifdef SSPLIB_ENABLE_EDITING
    XmlElement(const QDomElement& e);
#endif

    bool hasAttribute(const QString& name) const;
    QString attribute(const QString& name) const;
    QString tagName() const;

private:
    struct StreamElement
    {
        QXmlStreamAttributes attrs;
        QString tag;
    };

#ifdef SSPLIB_ENABLE_EDITING
    union {
        StreamElement streamElem;
        QDomElement domElem;
    };
    bool isDom;
#else
    StreamElement streamElem;
#endif

};

} // namespace utils

} // namespace ssplib

#endif // XMLELEMENT_H
