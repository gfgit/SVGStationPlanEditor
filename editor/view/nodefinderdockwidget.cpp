#include "nodefinderdockwidget.h"

#include <QToolButton>
#include <QTableView>

#include <QBoxLayout>

#include "model/iobjectmodel.h"

#include <QMessageBox>

NodeFinderDockWidget::NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr),
    model(nullptr)
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    //Toolbar
    QHBoxLayout *trackButLay = new QHBoxLayout;

    addBut = new QToolButton;
    addBut->setText(tr("Add"));
    trackButLay->addWidget(addBut);

    editBut = new QToolButton;
    editBut->setText(tr("Edit"));
    trackButLay->addWidget(editBut);

    remBut = new QToolButton;
    remBut->setText(tr("Remove"));
    trackButLay->addWidget(remBut);

    trackButLay->addStretch();
    lay->addLayout(trackButLay);

    view = new QTableView;
    lay->addWidget(view);

    connect(addBut, &QToolButton::clicked, this, &NodeFinderDockWidget::onAddItem);
    connect(editBut, &QToolButton::clicked, this, &NodeFinderDockWidget::onEditItem);
    connect(remBut, &QToolButton::clicked, this, &NodeFinderDockWidget::onRemoveItem);

    setMinimumSize(100, 100);
}

void NodeFinderDockWidget::setModel(IObjectModel *m, const QString &text)
{
    if(model)
        disconnect(model, &IObjectModel::errorOccurred, this, &NodeFinderDockWidget::showErrMsg);

    model = m;
    connect(model, &IObjectModel::errorOccurred, this, &NodeFinderDockWidget::showErrMsg);
    view->setModel(model);
    setWindowTitle(text);
}

void NodeFinderDockWidget::setDelegate(int col, QAbstractItemDelegate *delegate)
{
    view->setItemDelegateForColumn(col, delegate);
}

void NodeFinderDockWidget::onAddItem()
{
    model->addItem();
}

void NodeFinderDockWidget::onEditItem()
{
    if(!view->selectionModel()->hasSelection())
        return;

    QModelIndex idx = view->currentIndex();
    if(!idx.isValid())
        return;

    model->editItem(idx.row());
}

void NodeFinderDockWidget::onRemoveItem()
{
    if(!view->selectionModel()->hasSelection())
        return;

    QModelIndex idx = view->currentIndex();
    if(!idx.isValid())
        return;

    model->removeItem(idx.row());
}

void NodeFinderDockWidget::showErrMsg(const QString &msg)
{
    QMessageBox::warning(this, tr("Model Error"), msg);
}
