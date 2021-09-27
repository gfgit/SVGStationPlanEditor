#include "svgutils.h"

#include "nodefinderutils.h"

#include <QTextStream>

int parseNumber(double &outVal, const QStringView &str)
{
    //Calc number length
    int i = 0;
    bool isExponential = false;
    const QChar *c = str.data();
    for(; c && !c->isNull(); i++, c++)
    {
        if(c->isDigit() || *c == '.' || *c == '-')
            continue; //Still number

        if(c->toLower() == 'e')
        {
            isExponential = true;
            continue;
        }

        if(c->isSpace() || *c == ',')
            break; //Separator

        //Unexpected char
        return false;
    }
    if(i == 0)
        return false;

    bool ok = false;
    if(isExponential)
    {
        //Parse exponential notation 1.5e-1 -> 0.15
        QString tmp = str.left(i).toString();
        QTextStream stream(&tmp);
        stream >> outVal;
        ok = stream.status() == QTextStream::Ok;
    }
    else
    {
        outVal = str.left(i).toDouble(&ok);
    }

    return ok ? i : -1;
}

bool utils::parseNumberAndAdvance(double &outVal, QStringRef &str)
{
    //Calc number length
    int i = parseNumber(outVal, str);
    if(i < 0)
        return false;

    str = str.mid(i).trimmed();
    return true;
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
        double x1 = 0;
        if(!parseNumber(x1, str))
            return false;

        str = e.attribute(QLatin1String("y1"));
        if(str.isEmpty())
            return false;
        double y1 = 0;
        if(!parseNumber(y1, str))
            return false;

        str = e.attribute(QLatin1String("x2"));
        if(str.isEmpty())
            return false;
        double x2 = 0;
        if(!parseNumber(x2, str))
            return false;

        str = e.attribute(QLatin1String("y2"));
        if(str.isEmpty())
            return false;
        double y2 = 0;
        if(!parseNumber(y2, str))
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

int parseInteger(const QString& str, int &pos)
{
    int val = 0;
    for(; pos < str.size(); pos++)
    {
        if(str.at(pos).isDigit())
        {
            val *= 10;
            val += str.at(pos).digitValue();
        }

        if(str.at(pos) == ',' || str.at(pos) == ')')
            break;
    }
    return val;
}

bool utils::parseTrackConnectionAttribute(const QString &value, QVector<TrackConnectionInfo> &outVec)
{
    TrackConnectionInfo info;

    enum class Section
    {
        OutsideValue,
        GateLetter,
        GateTrack,
        StationTrack
    };

    Section section = Section::OutsideValue;

    for(int i = 0; i < value.size(); i++)
    {
        //Skip spaces and commas
        while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
        {
            i++;
        }

        if(i >= value.size())
            break;

        switch (section)
        {
        case Section::OutsideValue:
        {
            if(value.at(i) == '(')
            {
                section = Section::GateLetter;
            }

            break;
        }
        case Section::GateLetter:
        {
            if(value.at(i).isLetter())
            {
                info.gateLetter = value.at(i).toUpper();
                section = Section::GateTrack;
            }

            break;
        }
        case Section::GateTrack:
        {
            int num = parseInteger(value, i);
            info.gateTrackPos = num;
            section = Section::StationTrack;

            break;
        }
        case Section::StationTrack:
        {
            int num = parseInteger(value, i);
            info.stationTrackPos = num;

            //Skip spaces and commas
            while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
            {
                i++;
            }

            if(i < value.size() && value.at(i) == ')')
            {
                //Store value and restart parsing
                outVec.append(info);
                section = Section::OutsideValue;
            }

            break;
        }
        }
    }

    return true;
}

QString utils::trackConnInfoToString(const QVector<TrackConnectionInfo> &vec)
{
    QString value;
    value.reserve(8 * vec.size());

    for(const TrackConnectionInfo& info : vec)
    {
        value += '(';
        value += info.gateLetter;
        value += ',';
        value += QString::number(info.gateTrackPos);
        value += ',';
        value += QString::number(info.stationTrackPos);
        value += "),";
    }
    if(!value.isEmpty() && value.endsWith(','))
        value.chop(1); //Remove last comma

    return value;
}
