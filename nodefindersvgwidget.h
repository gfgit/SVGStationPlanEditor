#ifndef NODEFINDERSVGWIDGET_H
#define NODEFINDERSVGWIDGET_H

#include <QWidget>

class NodeFinderMgr;

class NodeFinderSVGWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderSVGWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

private:
    NodeFinderMgr *nodeMgr;
};

#endif // NODEFINDERSVGWIDGET_H
