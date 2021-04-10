#ifndef NODEFINDERSTATUSWIDGET_H
#define NODEFINDERSTATUSWIDGET_H

#include <QWidget>

class QLabel;
class QToolButton;

class NodeFinderMgr;

//FIXME: add zoom and trackPenWidth sliders
class NodeFinderStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderStatusWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

private slots:
    void setMode(int mode);

private:
    NodeFinderMgr *nodeMgr;

    QLabel *modeLabel;
    QToolButton *selectElemBut;
    QToolButton *nextElemBut;
};

#endif // NODEFINDERSTATUSWIDGET_H
