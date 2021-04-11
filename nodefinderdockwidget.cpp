#include "nodefinderdockwidget.h"

#include <QToolButton>
#include <QTableView>
#include <QScrollArea>

#include <QBoxLayout>

#include "nodefinderstationtracksmodel.h"

NodeFinderDockWidget::NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{

    QWidget *scrollAreaContents = new QWidget;
    QVBoxLayout *scrollLay = new QVBoxLayout(scrollAreaContents);

    //Labels
    labelsView = new QTableView;
    scrollLay->addWidget(labelsView);

    //Station Tracks
    QHBoxLayout *trackButLay = new QHBoxLayout;

    addTrackBut = new QToolButton;
    addTrackBut->setText(tr("Add Track"));
    trackButLay->addWidget(addTrackBut);

    editTrackBut = new QToolButton;
    editTrackBut->setText(tr("Edit Track"));
    trackButLay->addWidget(editTrackBut);

    remTrackBut = new QToolButton;
    remTrackBut->setText(tr("Remove Track"));
    trackButLay->addWidget(remTrackBut);

    trackButLay->addStretch();
    scrollLay->addLayout(trackButLay);

    tracksView = new QTableView;
    scrollLay->addWidget(tracksView);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(scrollAreaContents);
    mainLay->addWidget(scrollArea);
}

void NodeFinderDockWidget::setModels(QAbstractItemModel *labels, QAbstractItemModel *tracks)
{
    labelsView->setModel(labels);
    tracksView->setModel(tracks);
}

void NodeFinderDockWidget::onAddTrack()
{
    //FIXME: bad code
    static_cast<NodeFinderStationTracksModel *>(tracksView->model())->addItem();
}

void NodeFinderDockWidget::onEditTrack()
{
    if(!tracksView->selectionModel()->hasSelection())
        return;

    QModelIndex idx = tracksView->currentIndex();
    if(!idx.isValid())
        return;

    //FIXME: bad code
    static_cast<NodeFinderStationTracksModel *>(tracksView->model())->editItemAt(idx.row());
}

void NodeFinderDockWidget::onRemoveTrack()
{
    if(!tracksView->selectionModel()->hasSelection())
        return;

    QModelIndex idx = tracksView->currentIndex();
    if(!idx.isValid())
        return;

    //FIXME: bad code
    static_cast<NodeFinderStationTracksModel *>(tracksView->model())->removeItem(idx.row());
}
