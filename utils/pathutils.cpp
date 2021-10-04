#include "pathutils.h"


bool cutPathAtPoint(const QPointF &p, const QPainterPath &src, QPainterPath &dest, QPainterPath &rest)
{
    QPointF lastPoint;
    bool fakeLine = false;

    bool pastCutPoint = false;

    const int count = src.elementCount();
    for(int i = 0; i < count; i++)
    {
        QPainterPath::Element e = src.elementAt(i);

        if(fakeLine)
        {
            e.type = QPainterPath::LineToElement;
            fakeLine = false;
        }

        switch (e.type)
        {
        case QPainterPath::MoveToElement:
        {
            lastPoint = e;
            if(pastCutPoint)
                rest.moveTo(lastPoint);
            else
                dest.moveTo(lastPoint);
            break;
        }
        case QPainterPath::LineToElement:
        {
            QLineF line(lastPoint, e);

            QRectF rect(line.p1(), line.p2());
            if(!pastCutPoint && rect.contains(p))
            {
                //Cut
                pastCutPoint = true;

                //FIXME: we only consider X coord for cut
                //consider intersection of perpendicular from p to line

                double factor = (p.x() - line.x1()) / line.dx();
                QPointF end(p.x(), line.y1() + line.dy() * factor);

                //End destination path
                dest.lineTo(end);

                //Start remaining path
                rest.moveTo(end);
            }

            lastPoint = e;
            if(pastCutPoint)
                rest.lineTo(lastPoint);
            else
                dest.lineTo(lastPoint);
            break;
        }
        case QPainterPath::CurveToElement:
        {
            i++; //Skip second control point
            //Use end point as line end
            fakeLine = true;
            break;
        }
        case QPainterPath::CurveToDataElement:
        {
            break;
        }
        }
    }

    return true;
}
