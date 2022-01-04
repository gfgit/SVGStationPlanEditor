#ifndef NODEFINDERDOCKWIDGET_H
#define NODEFINDERDOCKWIDGET_H

#include <QWidget>

#include "utils/nodefindereditingmodes.h"

class NodeFinderMgr;

class QToolButton;
class QTableView;
class IObjectModel;

class QAbstractItemDelegate;

class NodeFinderDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

    void setModel(IObjectModel *m, const QString& text);

    void setDelegate(int col, QAbstractItemDelegate *delegate);

private slots:
    void onAddItem();
    void onEditItem();
    void onRemoveItem();
    void showErrMsg(const QString& msg);

private:
    NodeFinderMgr *nodeMgr;

    QTableView *view;
    IObjectModel *model;

    QToolButton *addBut;
    QToolButton *editBut;
    QToolButton *remBut;
};

#endif // NODEFINDERDOCKWIDGET_H
