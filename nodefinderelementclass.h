#ifndef NODEFINDERELEMENTCLASS_H
#define NODEFINDERELEMENTCLASS_H

#include <QString>
#include <QDomElement>
#include <QHash>

class NodeFinderSVGConverter;

class NodeFinderElementClass
{
public:
    typedef QHash<QString, QDomElement> ElementHash;

    enum CallbackResult
    {
        ReturnCurrentElement,
        KeepSearching,
        AbortSearch
    };

    typedef std::function<CallbackResult(QDomElement& e)> Callback;

    static const QString idAttr;

    NodeFinderElementClass(const QString& tag, const QString& baseId);

    QString getTagName() const;

    bool preocessElement(QDomElement e, NodeFinderSVGConverter *conv);

    void clear();

    inline bool getElementById(const QString& id, QDomElement &e) const
    {
        auto it = hash.constFind(id);
        if(it == hash.constEnd())
            return false;
        e = it.value();
        return true;
    }

    CallbackResult walkElements(QDomElement& result, Callback fun);

    void renameElement(QDomElement &e, const QString &newId, NodeFinderSVGConverter *conv);

private:
    QString tagName;
    QString m_baseId;
    int serial;

    ElementHash hash;
};

#endif // NODEFINDERELEMENTCLASS_H
