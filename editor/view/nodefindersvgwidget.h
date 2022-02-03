#ifndef NODEFINDERSVGWIDGET_H
#define NODEFINDERSVGWIDGET_H

#include <ssplib/rendering/sspviewer.h>

class NodeFinderMgr;

class NodeFinderSVGWidget : public ssplib::SSPViewer
{
    Q_OBJECT
public:
    explicit NodeFinderSVGWidget(ssplib::StationPlan *plan, NodeFinderMgr *mgr, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void keyPressEvent(QKeyEvent *e) override;

private:
    NodeFinderMgr *nodeMgr;
};

#endif // NODEFINDERSVGWIDGET_H
