#ifndef ELEMENTSPLITTERHELPER_H
#define ELEMENTSPLITTERHELPER_H

#include <QDomElement>
#include <QPointF>

class NodeFinderMgr;

class ElementSplitterHelper
{
public:
    ElementSplitterHelper(NodeFinderMgr *mgr, QDomElement e);

    bool splitAt(const QPointF& pos);

private:
    NodeFinderMgr *nodeMgr;
    QDomElement origElem;
};

#endif // ELEMENTSPLITTERHELPER_H
