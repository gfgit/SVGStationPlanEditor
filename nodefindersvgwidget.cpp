#include "nodefindersvgwidget.h"

#include <QSvgRenderer>

NodeFinderSVGWidget::NodeFinderSVGWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{

}

void NodeFinderSVGWidget::setRenderer(QSvgRenderer *svg)
{
    mSvg = svg;
}
