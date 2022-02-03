#ifndef ELEMENTSPLITTERHELPER_H
#define ELEMENTSPLITTERHELPER_H

#include <QDomElement>
#include <QPointF>

class NodeFinderMgr;

class ElementSplitterHelper
{
public:
    ElementSplitterHelper(NodeFinderMgr *mgr, QDomElement e, double threshold);

    bool splitAt(const QPointF& pos);

private:
    NodeFinderMgr *nodeMgr;
    QDomElement origElem;
    double m_threshold;
};

#endif // ELEMENTSPLITTERHELPER_H
