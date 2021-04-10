#include "nodefindersvgconverter.h"

#include "nodefindermgr.h"

NodeFinderSVGConverter::NodeFinderSVGConverter(NodeFinderMgr *parent) :
    QObject(parent),
    nodeMgr(parent)
{

}
