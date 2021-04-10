#ifndef NODEFINDERDOCKWIDGET_H
#define NODEFINDERDOCKWIDGET_H

#include <QWidget>

class NodeFinderMgr;

class QTableView;
class QAbstractItemModel;

class NodeFinderDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

    void setModels(QAbstractItemModel *labels, QAbstractItemModel *tracks);

private:
    NodeFinderMgr *nodeMgr;

    QTableView *labelsView;
    QTableView *tracksView;
};

#endif // NODEFINDERDOCKWIDGET_H
