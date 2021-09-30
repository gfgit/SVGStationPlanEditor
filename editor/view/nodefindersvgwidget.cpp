#include "nodefindersvgwidget.h"

#include "editor/manager/nodefindermgr.h"
#include "editor/manager/nodefindersvgconverter.h"

#include "editor/model/nodefinderlabelmodel.h"
#include "editor/model/nodefinderstationtracksmodel.h"

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
    static constexpr double PenWidthFactor = 1.5;

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
    trackPen.setCapStyle(Qt::RoundCap);

    //Draw labels
    if(nodeMgr->shouldDrawLabels())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();
        QFont f;
        const QString fmt = QLatin1String("Label %1");

        IObjectModel *labels = conv->getModel(EditingModes::LabelEditing);
        if(labels)
        {
            const int count = labels->getItemCount();
            for(int i = 0; i < count; i++)
            {
                const ItemBase *item = labels->getItemAt(i);
                if(!item->visible || item->elements.isEmpty())
                    continue; //Skip it

                for(const auto& elem : qAsConst(item->elements))
                {
                    //Do not draw path for labels

                    const QRectF r = elem.path.boundingRect();

                    //FIXME: bad code, use name getter func
                    const QString text = fmt.arg(static_cast<const LabelItem *>(item)->gateLetter);

                    int sizeH = r.height() * 0.85;
                    int sizeW = r.width() * 0.3;
                    const int minPixelSize = 10;

                    f.setPixelSize(qMax(minPixelSize, qMin(sizeH, sizeW)));
                    p.setFont(f);
                    p.drawText(r, text, QTextOption(Qt::AlignCenter));
                }

            }
        }
    }

    //Draw tracks
    if(nodeMgr->shouldDrawStationTracks())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();

        IObjectModel *tracks = conv->getModel(EditingModes::StationTrackEditing);
        if(tracks)
        {
            const int count = tracks->getItemCount();

            for(int i = 0; i < count; i++)
            {
                const ItemBase *item = tracks->getItemAt(i);
                if(!item->visible || item->elements.isEmpty())
                    continue; //Skip it

                for(const auto& elem : qAsConst(item->elements))
                {
                    if(elem.strokeWidth == 0)
                        trackPen.setWidthF(nodeMgr->getTrackPenWidth());
                    else
                        trackPen.setWidthF(elem.strokeWidth * PenWidthFactor);
                    p.setPen(trackPen);

                    p.drawPath(elem.path);
                }
            }
        }

        IObjectModel *conns = conv->getModel(EditingModes::TrackPathEditing);
        if(conns)
        {
            const int count = conns->getItemCount();

            for(int i = 0; i < count; i++)
            {
                const ItemBase *item = conns->getItemAt(i);
                if(!item->visible || item->elements.isEmpty())
                    continue; //Skip it

                for(const auto& elem : qAsConst(item->elements))
                {
                    if(elem.strokeWidth == 0)
                        trackPen.setWidthF(nodeMgr->getTrackPenWidth());
                    else
                        trackPen.setWidthF(elem.strokeWidth * PenWidthFactor);
                    p.setPen(trackPen);

                    p.drawPath(elem.path);
                }
            }
        }
    }

    //Draw selected item
    ElementPath curPath = nodeMgr->getConverter()->getCurElementPath();
    if(nodeMgr->getConverter()->getCurItem() || !curPath.path.isEmpty())
    {
        ItemBase *item = nodeMgr->getConverter()->getCurItem();
        const int itemSubIdx = nodeMgr->getConverter()->getCurItemSubElemIdx();

        //Draw selected item in blue or current element in red
        QColor color(curPath.path.isEmpty() ? Qt::blue : Qt::red);

        QPen pen;
        if(nodeMgr->mode() == EditingModes::StationTrackEditing || nodeMgr->mode() == EditingModes::TrackPathEditing)
        {
            pen = trackPen;
            pen.setColor(color);
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

    p.resetTransform();

    mSvg->render(&p, "text_layer", target);
    mSvg->render(&p, "arrows_layer", target);

    p.setTransform(transform);

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

void NodeFinderSVGWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        nodeMgr->selectCurrentElem();
        break;
    }
    case Qt::Key_Right:
    {
        nodeMgr->goToNextElem();
        break;
    }
    case Qt::Key_Left:
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
