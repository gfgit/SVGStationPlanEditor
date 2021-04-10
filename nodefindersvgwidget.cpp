#include "nodefindersvgwidget.h"

#include "nodefindermgr.h"
#include "nodefindersvgconverter.h"
#include "nodefinderlabelmodel.h"
#include "nodefinderstationtracksmodel.h"

#include <QSvgRenderer>

#include <QPainter>

NodeFinderSVGWidget::NodeFinderSVGWidget(NodeFinderMgr *mgr, QWidget *parent) :
    QWidget(parent),
    nodeMgr(mgr)
{

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


    QFont f;
    p.setFont(f);

    QPen pen = p.pen();
    QPen highLightPen = pen;
    highLightPen.setColor(Qt::red);

    const QString fmt = QLatin1String("Label %1");
    QColor color(Qt::blue);
    color.setAlpha(100);

    if(nodeMgr->shouldDrawLabels())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();
        for(const NodeFinderLabelModel::LabelItem& item : qAsConst(conv->labelsModel->items))
        {
            if(!item.visible || item.rect.isNull())
                continue; //Skip it

            p.fillRect(item.rect, color);

            f.setPixelSize(item.rect.height());
            p.setFont(f);

            QString text = fmt.arg(item.gateLetter);
            p.drawText(item.rect, text, QTextOption(Qt::AlignCenter));
        }
    }

    if(nodeMgr->shouldDrawStationTracks())
    {
        NodeFinderSVGConverter *conv = nodeMgr->getConverter();
        QPen trackPen(Qt::darkGreen, nodeMgr->getTrackPenWidth());
        p.setPen(trackPen);

        for(const NodeFinderStationTracksModel::TrackItem& item : qAsConst(conv->tracksModel->items))
        {
            if(!item.visible || item.path.isEmpty())
                continue; //Skip it

            p.drawPath(item.path);
        }
    }

    //TODO: draw selection rect in scaled coordinates so it scales with zoom changing
}
