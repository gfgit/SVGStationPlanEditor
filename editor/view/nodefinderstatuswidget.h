#ifndef NODEFINDERSTATUSWIDGET_H
#define NODEFINDERSTATUSWIDGET_H

#include <QWidget>

class QLabel;
class QToolButton;
class QSlider;

class NodeFinderMgr;

//FIXME: add zoom and trackPenWidth sliders
class NodeFinderStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NodeFinderStatusWidget(NodeFinderMgr *mgr, QWidget *parent = nullptr);

private slots:
    void updateMode();

private:
    NodeFinderMgr *nodeMgr;

    QLabel *modeLabel;

    QToolButton *splitElemBut;

    QToolButton *addSubElemBut;
    QToolButton *remSubElemBut;
    QToolButton *clearItemBut;

    QToolButton *selectElemBut;
    QToolButton *prevElemBut;
    QToolButton *nextElemBut;
    QToolButton *endEditBut;

    QSlider *trackPenWidthSlider;
};

#endif // NODEFINDERSTATUSWIDGET_H
