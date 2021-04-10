#include "nodefindermgr.h"

#include "nodefinderstatuswidget.h"
#include "nodefindersvgwidget.h"
#include "nodefinderdockwidget.h"

#include "nodefindersvgconverter.h"

NodeFinderMgr::NodeFinderMgr(QObject *parent) :
    QObject(parent)
{
    converter = new NodeFinderSVGConverter(this);
}

NodeFinderMgr::EditingModes NodeFinderMgr::mode() const
{
    return m_mode;
}

void NodeFinderMgr::setMode(const EditingModes &mode)
{
    m_mode = mode;
    emit modeChanged(int(m_mode));
}

QWidget *NodeFinderMgr::getStatusWidget(QWidget *parent)
{
    if(statusWidget && statusWidget->parent() == parent)
        return statusWidget;

    //Create a new one
    statusWidget = new NodeFinderStatusWidget(this, parent);
    return statusWidget;
}

QWidget *NodeFinderMgr::getCentralWidget(QWidget *parent)
{
    if(centralWidget && centralWidget->parent() == parent)
        return centralWidget;

    //Create a new one
    centralWidget = new NodeFinderSVGWidget(this, parent);
    return centralWidget;
}

QWidget *NodeFinderMgr::getDockWidget(QWidget *parent)
{
    if(dockWidget && dockWidget->parent() == parent)
        return dockWidget;

    //Create a new one
    dockWidget = new NodeFinderDockWidget(this, parent);
    return dockWidget;
}

bool NodeFinderMgr::loadSVG(QIODevice *dev)
{
    return false;
}

bool NodeFinderMgr::saveSVG(QIODevice *dev)
{
    return false;
}

void NodeFinderMgr::selectCurrentElem()
{

}

void NodeFinderMgr::goToNextElem()
{

}
