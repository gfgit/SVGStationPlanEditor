#include "nodefindersvgwidget.h"
#include <ssplib/rendering/ssprenderhelper.h>

#include "manager/nodefindermgr.h"
#include "manager/nodefindersvgconverter.h"

#include <QSvgRenderer>

#include <QPainter>

#include <QMouseEvent>

NodeFinderSVGWidget::NodeFinderSVGWidget(ssplib::StationPlan *plan, NodeFinderMgr *mgr, QWidget *parent) :
    ssplib::SSPViewer(plan, parent),
    nodeMgr(mgr)
{
    setBackgroundRole(QPalette::Light);
    setPlan(plan);
}

void NodeFinderSVGWidget::paintEvent(QPaintEvent *)
{
    static constexpr double PenWidthFactor = 1.5;

    const QRectF target = rect();
    const QRectF source = mSvg ? mSvg->viewBoxF() : target;

    QPen trackPen(Qt::darkGreen, nodeMgr->getTrackPenWidth());
    trackPen.setCapStyle(Qt::RoundCap);

    QPainter p(this);

    //Draw SVG image
    if(mSvg)
        mSvg->render(&p, target);

    //Draw visible items
    if(m_plan)
        ssplib::SSPRenderHelper::drawPlan(&p, m_plan, target, source);

    QTransform transform = ssplib::SSPRenderHelper::getTranform(target, source);
    p.setTransform(transform);

    //Draw selected item
    ssplib::ElementPath curPath = nodeMgr->getConverter()->getCurElementPath();
    if(nodeMgr->getConverter()->getCurItem() || !curPath.path.isEmpty())
    {
        ssplib::ItemBase *item = nodeMgr->getConverter()->getCurItem();
        const int itemSubIdx = nodeMgr->getConverter()->getCurItemSubElemIdx();

        //Draw selected item in blue or current element in red
        QColor color(curPath.path.isEmpty() ? Qt::blue : Qt::red);

        QPen pen;
        if(nodeMgr->mode() == EditingModes::StationTrackEditing || nodeMgr->mode() == EditingModes::TrackPathEditing)
        {
            pen = trackPen;
            pen.setColor(color);
            if(curPath.strokeWidth > 0)
                pen.setWidthF(curPath.strokeWidth * PenWidthFactor);
        }else{
            pen.setWidth(trackPen.width() / 2);
            p.setBrush(color);
        }
        p.setPen(pen);

        if(curPath.path.isEmpty())
        {
            //No current elem, draw item
            for(const auto& elem : qAsConst(item->elements))
            p.drawPath(elem.path);

            if(itemSubIdx >= 0 && itemSubIdx < item->elements.size())
            {
                //Highligt sub element of current item
                color = Qt::red;
                p.setBrush(color);
                pen.setColor(color);
                p.setPen(pen);

                p.drawPath(item->elements.at(itemSubIdx).path);
            }
        }
        else
        {
            //Draw current elem
            p.drawPath(curPath.path);
        }
    }

    //Draw selection rect
    QColor col(nodeMgr->isSelecting() ? Qt::red : Qt::green);
    col.setAlpha(50);
    p.fillRect(nodeMgr->getSelectionRect(), col);
}

void NodeFinderSVGWidget::mousePressEvent(QMouseEvent *e)
{
    if(!mSvg)
        return;

    if(e->button() == Qt::LeftButton)
    {
        const QRectF target = rect();
        const QRectF source = mSvg->viewBoxF();

        const double inverseScaleFactor = source.width() / target.width();
        const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

        nodeMgr->startSelection(pos);
    }
    else
    {
        nodeMgr->clearSelection();
    }
}

void NodeFinderSVGWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(!mSvg)
        return;

    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    nodeMgr->endOrMoveSelection(pos, false);
}

void NodeFinderSVGWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(!mSvg)
        return;

    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    nodeMgr->endOrMoveSelection(pos, true);
}

void NodeFinderSVGWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
    {
        nodeMgr->clearCurrentItem();
        break;
    }
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        nodeMgr->selectCurrentElem();
        break;
    }
    case Qt::Key_D:
    {
        nodeMgr->goToNextElem();
        break;
    }
    case Qt::Key_A:
    {
        nodeMgr->goToPrevElem();
        break;
    }
    default:
    {
        e->ignore();
        return;
    }
    }

    e->accept();
}
