#include "sspviewer.h"

#include <QSvgRenderer>

#include <QPainter>

#include "ssplib/stationplan.h"
#include "ssprenderhelper.h"

#include <QMouseEvent>
#include <QHelpEvent>
#include <QToolTip>

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

const ItemBase *SSPViewer::findItemAtPos(const QPointF &scenePos, FindItemType &outType) const
{
    //First try with labels
    for(const LabelItem& label : qAsConst(m_plan->labels))
    {
        for(const ElementPath& elem : label.elements)
        {
            const QRectF bounds = elem.path.boundingRect();
            if(bounds.contains(scenePos))
            {
                outType = FindItemType::Label;
                return &label;
            }
        }
    }

    //Then try with station tracks
    for(const TrackItem& track : qAsConst(m_plan->platforms))
    {
        for(const ElementPath& elem : track.elements)
        {
            const double halfWidth = elem.strokeWidth / 2;
            QRectF r(scenePos.x() - halfWidth, scenePos.y() - halfWidth, elem.strokeWidth, elem.strokeWidth);

            if(elem.path.intersects(r))
            {
                outType = FindItemType::StationTrack;
                return &track;
            }
        }
    }

    //Then try with track connections
    const TrackConnectionItem *possibleTrack = nullptr;
    for(const TrackConnectionItem& track : qAsConst(m_plan->trackConnections))
    {
        for(const ElementPath& elem : track.elements)
        {
            const double halfWidth = elem.strokeWidth / 2;
            QRectF r(scenePos.x() - halfWidth, scenePos.y() - halfWidth, elem.strokeWidth, elem.strokeWidth);

            if(elem.path.intersects(r))
            {
                if(possibleTrack)
                {
                    //Prefer visible track if we get multiple matches
                    if(!possibleTrack->visible && track.visible)
                        possibleTrack = &track;
                }
                else
                {
                    possibleTrack = &track;
                }
            }
        }
    }

    if(possibleTrack)
    {
        outType = FindItemType::TrackConnection;
        return possibleTrack;
    }

    outType = FindItemType::NotFound;
    return nullptr;
}

bool ssplib::SSPViewer::event(QEvent *e)
{
    if(e->type() == QEvent::ToolTip)
    {
        if(!mSvg || !m_plan)
        {
            QToolTip::hideText();
            e->ignore();
            return true;
        }

        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        const QRectF target = rect();
        const QRectF source = mSvg->viewBoxF();

        const double inverseScaleFactor = source.width() / target.width();
        const QPointF scenePos = helpEvent->pos() * inverseScaleFactor + source.topLeft();

        FindItemType itemType = FindItemType::NotFound;
        const ItemBase *item = findItemAtPos(scenePos, itemType);
        if(!item)
        {
            QToolTip::hideText();
            e->ignore();
            return true;
        }

        QString msg;
        if(itemType == FindItemType::Label)
        {
            const LabelItem *label = static_cast<const LabelItem *>(item);
            msg = tr("Gate: %1<br>"
                     "<b>%2</b><br>"
                     "Double click to get more details").arg(label->gateLetter).arg(label->labelText);
        }

        if(itemType == FindItemType::StationTrack)
        {
            const TrackItem *track = static_cast<const TrackItem *>(item);
            msg = track->jobName;
            if(msg.isEmpty() && !track->trackName.isEmpty())
                msg = tr("Platform: <b>%1</b>").arg(track->trackName);
        }

        if(itemType == FindItemType::TrackConnection)
        {
            const TrackConnectionItem *track = static_cast<const TrackConnectionItem *>(item);
            msg = track->jobName;
            if(msg.isEmpty() && !track->info.gateLetter.isNull())
            {
                msg = tr("Track connection to gate <b>%1</b>").arg(track->info.gateLetter);
            }
        }

        QToolTip::showText(helpEvent->globalPos(), msg, this);
        return true;
    }

    return QWidget::event(e);
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
    const QPointF scenePos = e->pos() * inverseScaleFactor + source.topLeft();

    FindItemType itemType = FindItemType::NotFound;
    const ItemBase *item = findItemAtPos(scenePos, itemType);
    if(!item)
        return;

    if(itemType == FindItemType::Label)
    {
        const LabelItem *label = static_cast<const LabelItem *>(item);
        e->accept();
        emit labelClicked(label->itemId, label->gateLetter, label->labelText);
        return;
    }

    if(itemType == FindItemType::StationTrack)
    {
        const TrackItem *track = static_cast<const TrackItem *>(item);
        e->accept();
        emit trackClicked(track->itemId, track->trackName);
        return;
    }

    if(itemType == FindItemType::TrackConnection)
    {
        const TrackConnectionItem *track = static_cast<const TrackConnectionItem *>(item);
        e->accept();
        emit trackConnClicked(track->itemId, track->info.trackId, track->info.gateId,
                              track->info.gateTrackPos, int(track->info.trackSide));
        return;
    }
}
