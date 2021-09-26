#ifndef NODEFINDERELEMENTCLASS_H
#define NODEFINDERELEMENTCLASS_H

#include <QString>
#include <QDomElement>
#include <QMap>

class NodeFinderSVGConverter;

class NodeFinderElementClass
{
public:
    typedef QHash<QString, QDomElement> ElementHash;
    typedef QMap<QString, QDomElement> ElementMap;

    static const QString idAttr;

    NodeFinderElementClass(const QString& tag, const QString& baseId);

    QString getTagName() const;

    bool preocessElement(QDomElement e, NodeFinderSVGConverter *conv);

    void clear();

    inline bool getElementById(const QString& id, QDomElement &e) const
    {
        auto it = elements.constFind(id);
        if(it == elements.constEnd())
            return false;
        e = it.value();
        return true;
    }

    void renameElement(QDomElement &e, const QString &newId, NodeFinderSVGConverter *conv);

private:
    QString tagName;
    QString m_baseId;
    int serial;

    friend class NodeFinderElementWalker;
    ElementMap elements;
};

#endif // NODEFINDERELEMENTCLASS_H
