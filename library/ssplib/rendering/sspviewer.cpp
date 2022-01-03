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
    e->ignore();

    if(!mSvg || !m_plan)
        return;

    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    //First try with labels
    bool found = false;
    for(const LabelItem& label : qAsConst(m_plan->labels))
    {
        for(const ElementPath& elem : label.elements)
        {
            const QRectF bounds = elem.path.boundingRect();
            if(bounds.contains(pos))
            {
                e->accept();
                emit labelClicked(label.itemId, label.gateLetter, label.labelText);
                found = true;
                break;
            }
        }

        if(found)
            break;
    }

    if(found)
        return;

    //Then try with station tracks
    for(const TrackItem& track : qAsConst(m_plan->platforms))
    {
        for(const ElementPath& elem : track.elements)
        {
            const double halfWidth = elem.strokeWidth / 2;
            QRectF r(pos.x() - halfWidth, pos.y() - halfWidth, elem.strokeWidth, elem.strokeWidth);

            if(elem.path.intersects(r))
            {
                e->accept();
                emit trackClicked(track.itemId, track.trackName);
                found = true;
                break;
            }
        }

        if(found)
            break;
    }

    if(found)
        return;

    //Then try with track connections
    for(const TrackConnectionItem& track : qAsConst(m_plan->trackConnections))
    {
        for(const ElementPath& elem : track.elements)
        {
            const double halfWidth = elem.strokeWidth / 2;
            QRectF r(pos.x() - halfWidth, pos.y() - halfWidth, elem.strokeWidth, elem.strokeWidth);

            if(elem.path.intersects(r))
            {
                e->accept();
                emit trackConnClicked(track.itemId, track.info.trackId, track.info.gateId,
                                      track.info.gateTrackPos, int(track.info.trackSide));
                found = true;
                break;
            }
        }

        if(found)
            break;
    }
}
