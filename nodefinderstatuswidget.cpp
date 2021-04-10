#include "nodefinderstatuswidget.h"

#include "nodefindermgr.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

NodeFinderStatusWidget::NodeFinderStatusWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    selectElemBut = new QToolButton;
    selectElemBut->setText(tr("Select"));
    selectElemBut->setToolTip(tr("Select current element"));
    lay->addWidget(selectElemBut);

    nextElemBut = new QToolButton;
    nextElemBut->setText(tr("Next"));
    nextElemBut->setToolTip(tr("Go to next element"));
    lay->addWidget(nextElemBut);

    modeLabel = new QLabel;
    modeLabel->setToolTip(tr("Editing mode"));
    lay->addWidget(modeLabel);

    connect(nodeMgr, &NodeFinderMgr::modeChanged, this, &NodeFinderStatusWidget::setMode);
    connect(selectElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::selectCurrentElem);
    connect(nextElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::goToNextElem);

    //Init with current mode
    setMode(int(nodeMgr->mode()));
}

void NodeFinderStatusWidget::setMode(int mode)
{
    NodeFinderMgr::EditingModes modeVal = NodeFinderMgr::EditingModes(mode);

    QString modeName;
    switch (modeVal)
    {
    case NodeFinderMgr::EditingModes::LabelEditing:
        modeName = tr("Label Editing");
        break;
    case NodeFinderMgr::EditingModes::StationTrackEditing:
        modeName = tr("Station Track Editing");
        break;
    case NodeFinderMgr::EditingModes::TrackPathEditing:
        modeName = tr("Track Path Editing");
        break;
    default:
        modeName = tr("Unknown mode");
    }

    modeLabel->setText(modeName);
}
