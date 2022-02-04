#ifndef ELEMENTSPLITTERHELPER_H
#define ELEMENTSPLITTERHELPER_H

#include <QDomElement>
#include <QPointF>

class QPainterPath;

class NodeFinderMgr;

class ElementSplitterHelper
{
public:
    ElementSplitterHelper(NodeFinderMgr *mgr, QDomElement e, double threshold);

    bool splitAt(const QPointF& pos);

    static bool convertToPath(QDomElement &e, const QPainterPath& path, NodeFinderMgr *mgr);

private:
    NodeFinderMgr *nodeMgr;
    QDomElement origElem;
    double m_threshold;
};

#endif // ELEMENTSPLITTERHELPER_H
