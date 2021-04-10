#include "nodefinderdockwidget.h"

#include <QTableView>
#include <QScrollArea>

#include <QVBoxLayout>

NodeFinderDockWidget::NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{

    QWidget *scrollAreaContents = new QWidget;
    QVBoxLayout *scrollLay = new QVBoxLayout(scrollAreaContents);

    labelsView = new QTableView;
    scrollLay->addWidget(labelsView);

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
