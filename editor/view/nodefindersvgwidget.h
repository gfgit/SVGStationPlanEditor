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

    QSize sizeHint() const override;

    void setRenderer(QSvgRenderer *svg);

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    NodeFinderMgr *nodeMgr;

    QSvgRenderer *mSvg;
};

#endif // NODEFINDERSVGWIDGET_H
