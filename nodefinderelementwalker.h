#ifndef NODEFINDERELEMENTWALKER_H
#define NODEFINDERELEMENTWALKER_H

#include "nodefinderelementclass.h"
#include <QHash>

class NodeFinderElementWalker
{
private:
    friend class NodeFinderSVGConverter;
    NodeFinderElementWalker(const QStringList& orderedTags, const QHash<QString, NodeFinderElementClass>& elementClasses);

public:
    inline NodeFinderElementWalker() : m_currentMap(nullptr), m_tagIdx(-1) {}

    bool next();
    bool prev();
    bool isValid() const;

    QDomElement element();

private:
    QStringList m_orderedTags;
    QHash<QString, NodeFinderElementClass> m_elementClasses;
    NodeFinderElementClass::ElementMap *m_currentMap;
    NodeFinderElementClass::ElementMap::iterator m_iter;
    int m_tagIdx;
};

#endif // NODEFINDERELEMENTWALKER_H
