#ifndef NODEFINDERDOCKWIDGET_H
#define NODEFINDERDOCKWIDGET_H

#include <QWidget>

class NodeFinderMgr;

class NodeFinderDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderDockWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

private:
    NodeFinderMgr *nodeMgr;
};

#endif // NODEFINDERDOCKWIDGET_H
