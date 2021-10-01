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
    inline NodeFinderElementWalker() = default;

    bool next();
    bool prev();
    bool isValid() const;

    QDomElement element();

    typedef struct Status
    {
        NodeFinderElementClass::ElementMap *currentMap = nullptr;
        NodeFinderElementClass::ElementMap::iterator iter;
        int tagIdx = -1;
    } Status;

    inline Status getStatus() const { return m_status; }
    inline void restoreStatus(const Status& s) { m_status = s; }

private:
    QStringList m_orderedTags;
    QHash<QString, NodeFinderElementClass> m_elementClasses;
    Status m_status;
};

#endif // NODEFINDERELEMENTWALKER_H
