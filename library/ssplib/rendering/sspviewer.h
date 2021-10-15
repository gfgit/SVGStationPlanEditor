#ifndef SSPLIB_SSPVIEWER_H
#define SSPLIB_SSPVIEWER_H

#include <QWidget>

class QSvgRenderer;

namespace ssplib {

class StationPlan;

class SSPViewer : public QWidget
{
    Q_OBJECT

public:
    explicit SSPViewer(StationPlan *plan, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void setRenderer(QSvgRenderer *svg);

    void setPlan(StationPlan *newPlan);

protected:
    void paintEvent(QPaintEvent *) override;

protected:
    StationPlan *m_plan;

    QSvgRenderer *mSvg;
};

} // namespace ssplib

#endif // SSPLIB_SSPVIEWER_H
