#include "svg_path_utils.h"

#include "svg_constants.h"

#include <QTextStream>

#include <QDebug>

using namespace ssplib;

const char trackSideLetters[int(ssplib::Side::NSides)] = {
    'W', //ssplib::Side::West
    'E' //ssplib::Side::East
};

static int parseNumber(double &outVal, const QStringView &str)
{    
    //Calc number length
    int i = 0;
    int start = 0;
    bool isExponential = false;
    bool insideNumber = false;
    const QChar *c = str.data();
    for(; c && !c->isNull(); i++, c++)
    {
        if(!insideNumber && c->isSpace())
            continue; //Skip leading spaces

        if(*c == '-' || *c == '+')
        {
            if(insideNumber)
            {
                //We are already inside number
                //A second sign is allowed only after 'e' in exponent
                //Check if previous char is 'e'
                if(i > 0)
                {
                    QChar prevCh = *(c - 1);
                    if(prevCh == 'e')
                        continue;
                }

                //Not exponent sign, it's unexpected so break
                break;
            }

            //Process sign at start of number
            insideNumber = true;
            start = i;
            continue;
        }

        if(c->toLower() == 'e')
        {
            if(!insideNumber || isExponential)
                break; //Already found 'e' or 'e' is not preceeded by numbers

            //First time we find 'e'
            isExponential = true;
            continue;
        }

        if(c->isDigit() || *c == '.')
        {
            if(!insideNumber)
            {
                start = i;
                insideNumber = true;
            }
            continue; //Still number
        }

        //Do not consider group separators (comma) because they should not be written to SVG
        //If we get other char, our number is terminated, so break
        break;
    }

    //Cut only number part and skip the rest to avoid confusing toDouble()
    QStringView numberStr = str.mid(start, i - start);
    bool ok = false;
    outVal = numberStr.toDouble(&ok);
    if(!ok)
        return -1;
    return i;
}

static bool parseNumberAndAdvanceRelative(double &outNum, QStringView &str, bool isRelative, const double prev)
{
    if(!utils::parseNumberAndAdvance(outNum, str))
        return false;

    if(isRelative)
    {
        outNum += prev;
    }

    return true;
}

static bool parsePointAndAdvanceRelative(QPointF &outPoint, QStringView &str, bool isRelative, const QPointF& prev)
{
    if(!utils::parsePointAndAdvance(outPoint, str))
        return false;

    if(isRelative)
    {
        outPoint += prev;
    }

    return true;
}

static int parseInteger(const QString& str, int &pos)
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

static bool convertLine(const utils::XmlElement &e, QPainterPath &path)
{
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

static bool convertPolyline(const utils::XmlElement &e, QPainterPath &path)
{
    QString str = e.attribute(QLatin1String("points"));
    if(str.isEmpty())
        return false;

    QStringView strRef(str);
    strRef = strRef.trimmed();
    QPointF pt;

    //Origin
    if(!utils::parsePointAndAdvance(pt, strRef))
        return false;
    path.moveTo(pt);

    //First line
    if(!utils::parsePointAndAdvance(pt, strRef))
        return false;
    path.lineTo(pt);

    while (utils::parsePointAndAdvance(pt, strRef))
    {
        path.lineTo(pt);
    }

    return true;
}

static bool convertPath(const utils::XmlElement &e, QPainterPath &path)
{
    QString str = e.attribute(QLatin1String("d"));
    if(str.isEmpty())
        return false;

    QStringView strRef(str);

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
    if(!utils::parsePointAndAdvance(pt, strRef))
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
            //Move to
            if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                return false;
            path.moveTo(pt);
            break;
        }
        case 'L':
        {
            //Line to
            if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                return false;
            path.lineTo(pt);
            break;
        }
        case 'H':
        {
            //Horizontal line
            double newX = 0;
            if(!parseNumberAndAdvanceRelative(newX, strRef, isRelative, prevPt.x()))
                return false;
            pt.setX(newX);
            path.lineTo(pt);
            break;
        }
        case 'V':
        {
            //Vertical line
            double newY = 0;
            if(!parseNumberAndAdvanceRelative(newY, strRef, isRelative, prevPt.y()))
                return false;
            pt.setY(newY);
            path.lineTo(pt);
            break;
        }
        case 'C':
        {
            //Cubic Bezier curve (x1 y1, x2 y2, x y)
            QPointF controlPt1, controlPt2;
            if(!parsePointAndAdvanceRelative(controlPt1, strRef, isRelative, prevPt))
                return false;
            if(!parsePointAndAdvanceRelative(controlPt2, strRef, isRelative, prevPt))
                return false;
            if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                return false;

            path.cubicTo(controlPt1, controlPt2, pt);
            break;
        }
        case 'Q':
        {
            //Quadratic Bezier curve (x1 y1, x y)
            QPointF controlPt1;
            if(!parsePointAndAdvanceRelative(controlPt1, strRef, isRelative, prevPt))
                return false;
            if(!parsePointAndAdvanceRelative(pt, strRef, isRelative, prevPt))
                return false;

            path.quadTo(controlPt1, pt);
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

static bool convertRect(const utils::XmlElement &e, QPainterPath &path)
{
    //height="27.528757" x="7.7508163" y="71.160507" width="57.195679"

    double x = 0, y = 0, w = 0, h = 0;

    QString str = e.attribute(QLatin1String("x"));
    if(str.isEmpty() || !parseNumber(x, str))
        return false;

    str = e.attribute(QLatin1String("y"));
    if(str.isEmpty() || !parseNumber(y, str))
        return false;

    str = e.attribute(QLatin1String("width"));
    if(str.isEmpty() || !parseNumber(w, str))
        return false;

    str = e.attribute(QLatin1String("height"));
    if(str.isEmpty() || !parseNumber(h, str))
        return false;

    path.addRect(x, y, w, h);

    return true;
}

bool utils::parseNumberAndAdvance(double &outVal, QStringView &str)
{
    //Calc number length
    int i = parseNumber(outVal, str);
    if(i < 0)
        return false;

    str = str.mid(i).trimmed();
    return true;
}

bool utils::parsePointAndAdvance(QPointF &outPoint, QStringView &str)
{
    //Points are separated by spaces
    //Coordinates are separated by spaces or comma or both

    // X
    if(!parseNumberAndAdvance(outPoint.rx(), str))
        return false;

    if(str.isEmpty())
        return false; //We expected also Y coordinate

    if(str.at(0) == ',')
        str = str.mid(1); //Eat comma

    str = str.trimmed();

    // Y
    if(!parseNumberAndAdvance(outPoint.ry(), str))
        return false;

    return true;
}

bool utils::convertElementToPath(const utils::XmlElement &e, QPainterPath &path)
{
    if(e.tagName() == svg_tags::LineTag)
    {
        return convertLine(e, path);
    }
    if(e.tagName() == svg_tags::PolylineTag)
    {
        return convertPolyline(e, path);
    }
    if(e.tagName() == svg_tags::PathTag)
    {
        return convertPath(e, path);
    }
    if(e.tagName() == svg_tags::RectTag)
    {
        return convertRect(e, path);
    }

    //Unsupported element
    return false;
}

bool utils::parseTrackConnectionAttribute(const QString &value, QList<TrackConnectionInfo> &outVec)
{
    TrackConnectionInfo info;

    enum class Section
    {
        OutsideValue,
        GateLetter,
        GateTrack,
        StationTrack,
        StationTrackSide
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
            section = Section::StationTrackSide;

            //Skip spaces and commas
            while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
            {
                i++;
            }

            if(i < value.size() && value.at(i) == ')')
            {
                //NOTE: we ignore StationTrackSide if not present to keep compatibility
                //with old format
                //Store value and restart parsing
                outVec.append(info);
                section = Section::OutsideValue;
            }else{
                //NOTE: go back by 1 so next loop goes forward by 1
                //And gets to StationTrackSide to parse it.
                i--;
            }

            break;
        }
        case Section::StationTrackSide:
        {
            const char sideLetter = value.at(i).toUpper().toLatin1();
            if(sideLetter == trackSideLetters[int(ssplib::Side::East)])
            {
                info.trackSide = ssplib::Side::East;
            }
            else if(sideLetter == trackSideLetters[int(ssplib::Side::West)])
            {
                info.trackSide = ssplib::Side::West;
            }
            else
            {
                info.trackSide = ssplib::Side::NSides;
            }
            i++;

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

QString utils::trackConnInfoToString(const QList<TrackConnectionInfo> &vec)
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
        value += ',';
        const int trackSide = int(info.trackSide);
        if(trackSide >= int(ssplib::Side::NSides))
            value += '?';
        else
            value += trackSideLetters[trackSide];
        value += "),";
    }
    if(!value.isEmpty() && value.endsWith(','))
        value.chop(1); //Remove last comma

    return value;
}

static bool parseStyleAttr(const QString& style, QString& strokeStr)
{
    //Parse CSS style
    const int idx = style.indexOf("stroke-width");
    if(idx < 0)
        return false;

    const int start = style.indexOf(':', idx + 12);
    if(start < 0)
        return false;

    const int end = style.indexOf(';', start);
    int length = end - start - 1;
    if(length < 0)
        length = -1; //Take all string

    strokeStr = style.mid(start + 1, length);
    return true;
}

utils::ElementStyle utils::parseStrokeWidthStyle(const utils::XmlElement &e, const ElementStyle& parentStyle, const QRectF& bounds)
{
    ElementStyle elemStyle = parentStyle;
    bool isFromStyle = false;

    QString strokeWidth;
    if(e.hasAttribute("style"))
    {
        //Parse CSS style first
        QString style = e.attribute("style");
        if(parseStyleAttr(style, strokeWidth))
            isFromStyle = true;
    }
    else if(e.hasAttribute("stroke-width"))
    {
        //Parse SVG attribute as fallback
        strokeWidth = e.attribute("stroke-width").simplified();
    }

    if(!strokeWidth.isEmpty())
    {
        //Parse our stroke width
        double val = 0;
        QStringView ref(strokeWidth);
        if(parseNumberAndAdvance(val, ref))
        {
            if(ref.contains('%'))
            {
                //Width relative to element size
                QSizeF sz = bounds.size();
                double averageSize = (sz.width() + sz.height()) / 2.0;
                val = val / 100 * averageSize;
            }

            if(!qFuzzyIsNull(val) && val > 0)
            {
                //Set the stroke width we found
                if(isFromStyle)
                    elemStyle.styleStrokeWidth = val;
                else
                    elemStyle.normalStrokeWidth = val;
            }
        }
    }

    return elemStyle;
}

bool utils::parseStrokeWidth(const utils::XmlElement &e, const ElementStyle& parentStyle, const QRectF& bounds, double &outVal)
{
    ElementStyle elemStyle = parseStrokeWidthStyle(e, parentStyle, bounds);

    if(elemStyle.styleStrokeWidth < 0)
    {
        //Fallback to normal style
        if(elemStyle.normalStrokeWidth < 0)
            return false; //No stroke width could be parsed
        outVal = elemStyle.normalStrokeWidth;
    }else{
        outVal = elemStyle.styleStrokeWidth;
    }

    return true;
}

#ifdef SSPLIB_ENABLE_EDITING

bool utils::parseStrokeWidthRecursve(QDomElement &e, const QRectF &bounds, double &outVal)
{
    const int N_Parents = 5;

    QDomElement parentElem = e;

    QDomElement arr[N_Parents + 1];
    arr[0] = e;

    //Go back to max N parents
    int i = 0;
    for(; i < N_Parents; i++)
    {
        QDomElement p = parentElem.parentNode().toElement();
        if(p.isNull())
            break; //This node has no parent

        arr[i + 1] = p;
        parentElem = p;
    }

    ElementStyle parentStyle;

    //Loop in reverse order
    for(int j = i; j >= 0; j--)
    {
        parentStyle = parseStrokeWidthStyle(XmlElement(arr[j]), parentStyle, bounds);
    }

    if(parentStyle.styleStrokeWidth < 0)
    {
        //Fallback to normal style
        if(parentStyle.normalStrokeWidth < 0)
            return false; //No stroke width could be parsed
        outVal = parentStyle.normalStrokeWidth;
    }
    else
    {
        outVal = parentStyle.styleStrokeWidth;
    }

    return true;
}

bool utils::convertPathToSVG(const QPainterPath &path, QString &outD)
{
    QTextStream stream(&outD);
    stream.setRealNumberPrecision(9);

    const int count = path.elementCount();

    for(int i = 0; i < count; i++)
    {
        QPainterPath::Element e = path.elementAt(i);
        switch (e.type)
        {
        case QPainterPath::MoveToElement:
        {
            stream << "M " << e.x << ' ' << e.y << ' ';
            break;
        }
        case QPainterPath::LineToElement:
        {
            stream << "L " << e.x << ' ' << e.y << ' ';
            break;
        }
        case QPainterPath::CurveToElement:
        {
            //Start cubic, write first control point
            stream << "C " << e.x << ' ' << e.y << ' ';
            break;
        }
        case QPainterPath::CurveToDataElement:
        {
            //Write second control point or end point
            stream << e.x << ' ' << e.y << ' ';
            break;
        }
        }
    }

    if(outD.endsWith(' '))
        outD.chop(1); //Remove last space

    return true;
}

#endif // SSPLIB_ENABLE_EDITING
