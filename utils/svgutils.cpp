#include "svgutils.h"

#include "nodefinderutils.h"

bool utils::parseNumberAndAdvance(double &outVal, QStringRef &str)
{
    //Calc number length
    int i = 0;
    const QChar *c = str.data();
    for(; c && !c->isNull(); i++, c++)
    {
        if(c->isDigit() || *c == '.' || *c == '-')
            continue; //Still number

        if(c->isSpace() || *c == ',')
            break; //Separator

        //Unexpected char
        return false;
    }
    if(i == 0)
        return false;

    bool ok = false;
    outVal = str.left(i).toDouble(&ok);
    if(ok)
    {
        str = str.mid(i).trimmed();
        return true;
    }
    return false;
}

bool parseNumberAndAdvanceRelative(double &outNum, QStringRef &str, bool isRelative, const double prev)
{
    if(!utils::parseNumberAndAdvance(outNum, str))
        return false;

    if(isRelative)
    {
        outNum += prev;
    }

    return true;
}

bool utils::parsePointAndAdvance(QPointF &outPoint, QStringRef &str)
{
    //Points are separated by spaces
    //Coordinates are separated by spaces or comma or both

    // X
    if(!parseNumberAndAdvance(outPoint.rx(), str))
        return false;

    if(str.at(0) == ',')
        str = str.mid(1); //Eat comma

    str = str.trimmed();

    // Y
    if(!parseNumberAndAdvance(outPoint.ry(), str))
        return false;

    return true;
}

bool parsePointAndAdvanceRelative(QPointF &outPoint, QStringRef &str, bool isRelative, const QPointF& prev)
{
    if(!utils::parsePointAndAdvance(outPoint, str))
        return false;

    if(isRelative)
    {
        outPoint += prev;
    }

    return true;
}

bool utils::convertElementToPath(const QDomElement &e, QPainterPath &path)
{
    if(e.tagName() == svg_tag::LineTag)
    {
        bool ok = false;
        QString str = e.attribute(QLatin1String("x1"));
        if(str.isEmpty())
            return false;
        double x1 = str.toDouble(&ok);
        if(!ok)
            return false;

        str = e.attribute(QLatin1String("y1"));
        if(str.isEmpty())
            return false;
        double y1 = str.toDouble(&ok);
        if(!ok)
            return false;

        str = e.attribute(QLatin1String("x2"));
        if(str.isEmpty())
            return false;
        double x2 = str.toDouble(&ok);
        if(!ok)
            return false;

        str = e.attribute(QLatin1String("y2"));
        if(str.isEmpty())
            return false;
        double y2 = str.toDouble(&ok);
        if(!ok)
            return false;

        path.moveTo(x1, y1);
        path.lineTo(x2, y2);
        return true;
    }
    if(e.tagName() == svg_tag::PolylineTag)
    {
        QString str = e.attribute(QLatin1String("points"));
        if(str.isEmpty())
            return false;

        QStringRef strRef(&str);
        strRef = strRef.trimmed();
        QPointF pt;

        //Origin
        if(!parsePointAndAdvance(pt, strRef))
            return false;
        path.moveTo(pt);

        //First line
        if(!parsePointAndAdvance(pt, strRef))
            return false;
        path.lineTo(pt);

        while (parsePointAndAdvance(pt, strRef))
        {
            path.lineTo(pt);
        }

        return true;
    }
    if(e.tagName() == svg_tag::PathTag)
    {
        QString str = e.attribute(QLatin1String("d"));
        if(str.isEmpty())
            return false;

        QStringRef strRef(&str);

        //Skip leading spaces
        strRef = strRef.trimmed();

        bool defaultRelative = false;
        QChar startLetter = strRef.at(0);
        if(startLetter == 'M')
            defaultRelative = false;
        else if(startLetter == 'm')
            defaultRelative = true;
        else
            return false; //Must start with M or m

        //Eat letter and spaces
        strRef = strRef.mid(1).trimmed();

        QPointF pt;

        //Origin
        if(!parsePointAndAdvance(pt, strRef))
            return false;
        path.moveTo(pt);

        QPointF prevPt = pt;

        int i = 0;
        while (i < 1000)
        {
            if(strRef.isEmpty())
                break; //End cicle

            QChar op = strRef.at(0);
            if(!op.isLetter())
            {
                //Default to line to
                if(defaultRelative)
                    op = 'l';
                else
                    op = 'L';
            }
            else
            {
                //Eat letter
                strRef = strRef.mid(1);
            }

            //Eat spaces
            strRef = strRef.trimmed();

            const bool isRelative = op.isLower();

            //Switch on upper case, we already set correct point
            switch (op.toUpper().toLatin1())
            {
            case 'M':
            {
                if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                    return false;
                path.moveTo(pt);
                break;
            }
            case 'L':
            {
                if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                    return false;
                path.lineTo(pt);
                break;
            }
            case 'H':
            {
                double newX = 0;
                if(!parseNumberAndAdvanceRelative(newX, strRef, isRelative, prevPt.x()))
                    return false;
                pt.setX(newX);
                path.lineTo(pt);
                break;
            }
            case 'V':
            {
                double newY = 0;
                if(!parseNumberAndAdvanceRelative(newY, strRef, isRelative, prevPt.y()))
                    return false;
                pt.setY(newY);
                path.lineTo(pt);
                break;
            }
            default:
            {
                if(i == 0) //At least 1 line
                    return false;
                return true; //Ignore unsupported op
            }
            }

            prevPt = pt;

            i++;
        }

        if(i == 0) //At least 1 line
            return false;
        return true;
    }

    //Unsupported element
    return false;
}
