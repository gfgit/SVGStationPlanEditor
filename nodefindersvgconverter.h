#ifndef NODEFINDERSVGCONVERTER_H
#define NODEFINDERSVGCONVERTER_H

#include <QObject>

class NodeFinderMgr;

class NodeFinderSVGConverter : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderSVGConverter(NodeFinderMgr *parent);

private:
    NodeFinderMgr *nodeMgr;
};

#endif // NODEFINDERSVGCONVERTER_H
