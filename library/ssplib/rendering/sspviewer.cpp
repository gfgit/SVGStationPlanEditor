#include "sspviewer.h"

#include <QSvgRenderer>

#include <QPainter>

#include "ssplib/stationplan.h"
#include "ssprenderhelper.h"

#include <QMouseEvent>

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
    if(mSvg && mSvg->isValid())
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

void SSPViewer::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(!mSvg || !m_plan)
        return;

    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    for(const LabelItem& label : qAsConst(m_plan->labels))
    {
        for(const ElementPath& elem : label.elements)
        {
            const QRectF bounds = elem.path.boundingRect();
            if(bounds.contains(pos))
            {
                e->accept();
                emit labelClicked(label.itemId, label.gateLetter, label.labelText);
                break;
            }
        }
    }
}
