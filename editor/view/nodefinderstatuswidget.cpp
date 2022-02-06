#include "nodefinderstatuswidget.h"

#include "manager/nodefindermgr.h"

#include <QHBoxLayout>

#include <QLabel>
#include <QToolButton>
#include <QSlider>

NodeFinderStatusWidget::NodeFinderStatusWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    trackPenWidthSlider = new QSlider(Qt::Horizontal, this);
    trackPenWidthSlider->setRange(2, 400);
    trackPenWidthSlider->setToolTip(tr("Track Pen Width"));
    trackPenWidthSlider->setValue(nodeMgr->getTrackPenWidth());
    lay->addWidget(trackPenWidthSlider);

    splitElemBut = new QToolButton;
    splitElemBut->setText(tr("Split"));
    splitElemBut->setToolTip(tr("Start element selection and then split selected element at mouse click"));
    lay->addWidget(splitElemBut);

    addSubElemBut = new QToolButton;
    addSubElemBut->setText(tr("Add"));
    addSubElemBut->setToolTip(tr("Start element selection to add new sub element to current item"));
    lay->addWidget(addSubElemBut);

    remSubElemBut = new QToolButton;
    remSubElemBut->setText(tr("Remove"));
    remSubElemBut->setToolTip(tr("Start item's elements selection remove sub element from current item"));
    lay->addWidget(remSubElemBut);

    clearItemBut = new QToolButton;
    clearItemBut->setText(tr("Unselect"));
    clearItemBut->setToolTip(tr("Clear selection of current item"));
    lay->addWidget(clearItemBut);

    prevElemBut = new QToolButton;
    prevElemBut->setText(tr("Prev"));
    prevElemBut->setToolTip(tr("Go to prev element"));
    prevElemBut->setShortcut(Qt::Key_A);
    lay->addWidget(prevElemBut);

    selectElemBut = new QToolButton;
    selectElemBut->setShortcut(Qt::Key_S);
    lay->addWidget(selectElemBut);

    nextElemBut = new QToolButton;
    nextElemBut->setText(tr("Next"));
    nextElemBut->setToolTip(tr("Go to next element"));
    nextElemBut->setShortcut(Qt::Key_D);
    lay->addWidget(nextElemBut);

    endEditBut = new QToolButton;
    endEditBut->setText(tr("End"));
    endEditBut->setToolTip(tr("End editing"));
    lay->addWidget(endEditBut);

    modeLabel = new QLabel;
    modeLabel->setToolTip(tr("Editing mode"));
    lay->addWidget(modeLabel);

    connect(nodeMgr, &NodeFinderMgr::modeChanged, this, &NodeFinderStatusWidget::updateMode);

    connect(trackPenWidthSlider, &QSlider::valueChanged, nodeMgr, &NodeFinderMgr::setTrackPenWidth);
    connect(nodeMgr, &NodeFinderMgr::trackPenWidthChanged, trackPenWidthSlider, [this](int val)
            {
                trackPenWidthSlider->setToolTip(tr("Track Pen Width - %1").arg(val));
                trackPenWidthSlider->setValue(val);
            });

    connect(splitElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::startElementSplitProcess);
    connect(addSubElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::requestAddSubElement);
    connect(remSubElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::requestRemoveSubElement);
    connect(clearItemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::clearCurrentItem);

    connect(selectElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::selectCurrentElem);
    connect(prevElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::goToPrevElem);
    connect(nextElemBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::goToNextElem);
    connect(endEditBut, &QToolButton::clicked, nodeMgr, &NodeFinderMgr::requestEndEditItem);

    //Init with current mode
    updateMode();
}

void NodeFinderStatusWidget::updateMode()
{
    const EditingModes mode = nodeMgr->mode();
    const EditingSubModes subMode = nodeMgr->getSubMode();

    QString modeName = nodeMgr->getModeName(mode);

    switch (nodeMgr->getSubMode())
    {
    case EditingSubModes::AddingSubElement:
        modeName.append(tr(", ADD"));
        break;
    case EditingSubModes::RemovingSubElement:
        modeName.append(tr(", REM"));
        break;
    case EditingSubModes::DoSplitItem:
        modeName.append(tr(", CUT"));
        break;
    case EditingSubModes::NotEditingCurrentItem:
    case EditingSubModes::NSubModes:
        break;
    }

    modeLabel->setText(modeName);

    const bool isEditing = mode > EditingModes::NoEditing && mode < EditingModes::NModes;

    if(!isEditing)
    {
        splitElemBut->show();
        addSubElemBut->show();
        remSubElemBut->show();
        clearItemBut->show();

        addSubElemBut->setEnabled(false);
        remSubElemBut->setEnabled(false);
        clearItemBut->setEnabled(false);

        prevElemBut->hide();
        selectElemBut->hide();
        nextElemBut->hide();
        endEditBut->hide();
    }
    else
    {
        addSubElemBut->setEnabled(true);
        remSubElemBut->setEnabled(true);
        clearItemBut->setEnabled(true);

        const bool showEditControls = subMode != EditingSubModes::NotEditingCurrentItem;

        splitElemBut->setVisible(!showEditControls);
        addSubElemBut->setVisible(!showEditControls);
        remSubElemBut->setVisible(!showEditControls);
        clearItemBut->setVisible(!showEditControls);

        prevElemBut->setVisible(showEditControls);
        selectElemBut->setVisible(showEditControls);
        nextElemBut->setVisible(showEditControls);
        endEditBut->setVisible(showEditControls);

        if(subMode == EditingSubModes::AddingSubElement)
        {
            selectElemBut->setText(tr("Select"));
            selectElemBut->setToolTip(tr("Add element to current item"));
        }
        else if(subMode == EditingSubModes::DoSplitItem)
        {
            selectElemBut->setText(tr("Select"));
            selectElemBut->setToolTip(tr("Select element to be split"));
        }
        else if(subMode == EditingSubModes::RemovingSubElement)
        {
            selectElemBut->setText(tr("Remove"));
            selectElemBut->setToolTip(tr("Remove sub element"));
        }
    }
}
