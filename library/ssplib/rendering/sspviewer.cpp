#include "sspviewer.h"

#include <QSvgRenderer>

#include <QPainter>

#include "../stationplan.h"
#include "ssprenderhelper.h"

using namespace ssplib;

SSPViewer::SSPViewer(StationPlan *mgr, QWidget *parent) :
    QWidget(parent),
    m_plan(mgr),
    mSvg(nullptr)
{
    setBackgroundRole(QPalette::Light);
}

QSize SSPViewer::sizeHint() const
{
    if(mSvg->isValid())
        return mSvg->defaultSize();
    return QSize(128, 64);
}

void SSPViewer::setRenderer(QSvgRenderer *svg)
{
    mSvg = svg;
}

void SSPViewer::setPlan(StationPlan *newPlan)
{
    m_plan = newPlan;
}

void SSPViewer::paintEvent(QPaintEvent *)
{
    const QRectF target = rect();
    const QRectF source = mSvg ? mSvg->viewBoxF() : target;

    QPainter p(this);

    if(mSvg)
        mSvg->render(&p, target);

    if(m_plan)
        SSPRenderHelper::drawPlan(&p, m_plan, target, source);
}
