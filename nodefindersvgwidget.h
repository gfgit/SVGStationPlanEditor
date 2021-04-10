#ifndef NODEFINDERSVGWIDGET_H
#define NODEFINDERSVGWIDGET_H

#include <QWidget>

class QSvgRenderer;

class NodeFinderMgr;

class NodeFinderSVGWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderSVGWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

    void setRenderer(QSvgRenderer *svg);

private:
    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;
};

#endif // NODEFINDERSVGWIDGET_H
