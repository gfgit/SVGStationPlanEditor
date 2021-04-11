#include "nodefindersvgwidget.h"

#include "nodefindermgr.h"
#include "nodefindersvgconverter.h"
#include "nodefinderlabelmodel.h"
#include "nodefinderstationtracksmodel.h"

#include <QSvgRenderer>

#include <QPainter>

#include <QMouseEvent>

NodeFinderSVGWidget::NodeFinderSVGWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{
    setBackgroundRole(QPalette::Light);
}

QSize NodeFinderSVGWidget::sizeHint() const
{
    if(mSvg->isValid())
        return mSvg->defaultSize();
    return QSize(128, 64);
}

void NodeFinderSVGWidget::setRenderer(QSvgRenderer *svg)
{
    mSvg = svg;
}

void NodeFinderSVGWidget::paintEvent(QPaintEvent *)
{
    QRectF target = rect();
    QRectF source = mSvg->viewBoxF();

    QPainter p(this);
    mSvg->render(&p, target);

    const double scaleFactor = target.width() / source.width();

    QTransform transform;
    transform.scale(target.width() / source.width(),
                    target.height() / source.height());
    QRectF c2 = transform.mapRect(source);

    transform.reset();
    transform.translate(target.x() - c2.x(),
                        target.y() - c2.y());
    transform.scale(scaleFactor, scaleFactor);
    p.setTransform(transform);

    QPen trackPen(Qt::darkGreen, nodeMgr->getTrackPenWidth());

    //Draw labels
    if(nodeMgr->shouldDrawLabels())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();
        QFont f;
        const QString fmt = QLatin1String("Label %1");

        for(const LabelItem& item : qAsConst(conv->labelsModel->items))
        {
            if(!item.visible || item.elements.isEmpty())
                continue; //Skip it

            for(const auto& elem : qAsConst(item.elements))
            {
                //Do not draw path for labels

                const QRectF r = elem.path.boundingRect();
                const QString text = fmt.arg(item.gateLetter);

                f.setPixelSize(r.height() * 0.85);
                p.setFont(f);
                p.drawText(r, text, QTextOption(Qt::AlignCenter));
            }

        }
    }

    //Draw tracks
    if(nodeMgr->shouldDrawStationTracks())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();
        p.setPen(trackPen);

        for(const TrackItem& item : qAsConst(conv->tracksModel->items))
        {
            if(!item.visible || item.elements.isEmpty())
                continue; //Skip it

            for(const auto& elem : qAsConst(item.elements))
                p.drawPath(elem.path);
        }
    }

    //Draw selected item
    if(nodeMgr->getConverter()->getCurItem())
    {
        ItemBase *item = nodeMgr->getConverter()->getCurItem();
        const int itemSubIdx = nodeMgr->getConverter()->getCurItemSubElemIdx();

        //Draw selected item in blue
        QColor color(Qt::blue);
        p.setBrush(color);

        QPen pen;
        if(nodeMgr->mode() == EditingModes::StationTrackEditing || nodeMgr->mode() == EditingModes::TrackPathEditing)
        {
            pen = trackPen;
            pen.setColor(color);
        }
        p.setPen(pen);

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

    //Draw selection rect
    QColor col(nodeMgr->isSelecting() ? Qt::red : Qt::green);
    col.setAlpha(50);
    p.fillRect(nodeMgr->getSelectionRect(), col);
}

void NodeFinderSVGWidget::mousePressEvent(QMouseEvent *e)
{
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
    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    nodeMgr->endOrMoveSelection(pos, false);
}

void NodeFinderSVGWidget::mouseReleaseEvent(QMouseEvent *e)
{
    const QRectF target = rect();
    const QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    nodeMgr->endOrMoveSelection(pos, true);
}
