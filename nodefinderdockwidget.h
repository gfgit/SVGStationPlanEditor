#ifndef NODEFINDERDOCKWIDGET_H
#define NODEFINDERDOCKWIDGET_H

#include <QWidget>

class NodeFinderMgr;

class QToolButton;
class QTableView;
class QAbstractItemModel;

class NodeFinderDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

    void setModels(QAbstractItemModel *labels, QAbstractItemModel *tracks);

private slots:
    void onAddTrack();
    void onEditTrack();
    void onRemoveTrack();

private:
    NodeFinderMgr *nodeMgr;

    QTableView *labelsView;

    QTableView *tracksView;
    QToolButton *addTrackBut;
    QToolButton *editTrackBut;
    QToolButton *remTrackBut;
};

#endif // NODEFINDERDOCKWIDGET_H
