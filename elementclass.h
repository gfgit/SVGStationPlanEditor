#ifndef ELEMENTCLASS_H
#define ELEMENTCLASS_H

#include <QDomElement>
#include <QHash>

#include <functional>

class SVGTinyConverter;

class ElementClass
{
public:
    typedef QHash<QString, QDomElement> ElementHash;

    enum CallbackResult
    {
        ReturnCurrentElement,
        KeepSearching,
        AbortSearch
    };

    typedef std::function<CallbackResult(const QDomElement& e)> Callback;

    ElementClass(SVGTinyConverter *c, const QString& tag, const QString& baseId);

    QString getTagName() const;

    bool preocessElement(QDomElement e);

    void clear();

    inline bool getElementById(const QString& id, QDomElement &e) const
    {
        auto it = hash.constFind(id);
        if(it == hash.constEnd())
            return false;
        e = it.value();
        return true;
    }

    CallbackResult walkElements(QDomElement& result, Callback fun) const;

    void renameElement(QDomElement &e, const QString &newId);

private:
    SVGTinyConverter *conv;

    QString tagName;
    QString m_baseId;
    int serial;

    ElementHash hash;
};

#endif // ELEMENTCLASS_H
