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
    XmlElement(const QStringRef& tag, const QXmlStreamAttributes& attrs);

#ifdef SSPLIB_ENABLE_EDITING
    XmlElement(QDomElement& e);
#endif

    bool hasAttribute(const QString& name) const;
    QString attribute(const QString& name) const;
    QString tagName() const;

    void removeAttribute(const QString& name);

private:
    struct StreamElement
    {
        QXmlStreamAttributes attrs;
        QString tag;
    };

    StreamElement streamElem;

#ifdef SSPLIB_ENABLE_EDITING
    //FIXME: use union but be careful with constructors and destructors
    QDomElement domElem;
    bool isDom = false;
#endif

};

} // namespace utils

} // namespace ssplib

#endif // XMLELEMENT_H
