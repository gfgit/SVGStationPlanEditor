#include "nodefindermgr.h"

#include "nodefinderstatuswidget.h"
#include "nodefindersvgwidget.h"
#include "nodefinderdockwidget.h"

#include "nodefindersvgconverter.h"

NodeFinderMgr::NodeFinderMgr(QObject *parent) :
    QObject(parent),
    drawLabels(true),
    drawStationTracks(true),
    trackPenWidth(10)
{
    converter = new NodeFinderSVGConverter(this);

    setMode(EditingModes::NoEditing);
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
    NodeFinderSVGWidget *w = new NodeFinderSVGWidget(this, parent);
    w->setRenderer(converter->renderer());
    connect(this, &NodeFinderMgr::repaintSVG, w, QOverload<>::of(&QWidget::update));

    centralWidget = w;
    return centralWidget;
}

QWidget *NodeFinderMgr::getDockWidget(QWidget *parent)
{
    if(dockWidget && dockWidget->parent() == parent)
        return dockWidget;

    //Create a new one
    NodeFinderDockWidget *w = new NodeFinderDockWidget(this, parent);
    w->setModels(converter->getLabelsModel(), converter->getTracksModel());

    dockWidget = w;
    return dockWidget;
}

bool NodeFinderMgr::loadSVG(QIODevice *dev)
{
    bool ret = converter->load(dev);
    if(!ret)
    {
        converter->clear();
        setMode(EditingModes::NoSVGLoaded);
    }

    converter->processElements();
    converter->loadLabelsAndTracks();

    //This will also trigger repaint
    setTrackPenWidth(converter->calcDefaultTrackPenWidth());

    setMode(EditingModes::NoEditing);
    return true;
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

void NodeFinderMgr::setTrackPenWidth(int value)
{
    trackPenWidth = value;
    emit trackPenWidthChanged(trackPenWidth);
    emit repaintSVG();
}
