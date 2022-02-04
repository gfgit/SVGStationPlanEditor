#include "transform_utils.h"

#include "svg_path_utils.h"

#include <QtMath>

using namespace ssplib;

static inline const QChar *parseNumbersArray(const QStringRef &origStr, QVarLengthArray<qreal, 8> &points)
{
    QStringRef str = origStr;

    double val = 0;
    while (!str.isEmpty() && str.at(0) != ')')
    {   
        if(!utils::parseNumberAndAdvance(val, str))
            break;
        points.append(val);

        if(!str.isEmpty() && str.at(0) == ',')
            str = str.mid(1); //Skip comma
    }

    return str.trimmed().constData();
}


QMatrix ssplib::utils::parseTransformationMatrix(const QStringRef &value)
{
    if (value.isEmpty())
        return QMatrix();
    QMatrix matrix;
    const QChar *str = value.constData();
    const QChar *end = str + value.length();
    while (str < end) {
        if (str->isSpace() || *str == QLatin1Char(',')) {
            ++str;
            continue;
        }
        enum State {
            Matrix,
            Translate,
            Rotate,
            Scale,
            SkewX,
            SkewY
        };
        State state = Matrix;
        if (*str == QLatin1Char('m')) {  //matrix
            const char *ident = "atrix";
            for (int i = 0; i < 5; ++i)
                if (*(++str) != QLatin1Char(ident[i]))
                    goto error;
            ++str;
            state = Matrix;
        } else if (*str == QLatin1Char('t')) { //translate
            const char *ident = "ranslate";
            for (int i = 0; i < 8; ++i)
                if (*(++str) != QLatin1Char(ident[i]))
                    goto error;
            ++str;
            state = Translate;
        } else if (*str == QLatin1Char('r')) { //rotate
            const char *ident = "otate";
            for (int i = 0; i < 5; ++i)
                if (*(++str) != QLatin1Char(ident[i]))
                    goto error;
            ++str;
            state = Rotate;
        } else if (*str == QLatin1Char('s')) { //scale, skewX, skewY
            ++str;
            if (*str == QLatin1Char('c')) {
                const char *ident = "ale";
                for (int i = 0; i < 3; ++i)
                    if (*(++str) != QLatin1Char(ident[i]))
                        goto error;
                ++str;
                state = Scale;
            } else if (*str == QLatin1Char('k')) {
                if (*(++str) != QLatin1Char('e'))
                    goto error;
                if (*(++str) != QLatin1Char('w'))
                    goto error;
                ++str;
                if (*str == QLatin1Char('X'))
                    state = SkewX;
                else if (*str == QLatin1Char('Y'))
                    state = SkewY;
                else
                    goto error;
                ++str;
            } else {
                goto error;
            }
        } else {
            goto error;
        }
        while (str < end && str->isSpace())
            ++str;
        if (*str != QLatin1Char('('))
            goto error;
        ++str;
        QVarLengthArray<qreal, 8> points;
        str = parseNumbersArray(value.mid(str - value.constData()), points);
        if(!str)
            goto error;
        if (*str != QLatin1Char(')'))
            goto error;
        ++str;
        if(state == Matrix) {
            if(points.count() != 6)
                goto error;
            matrix = QMatrix(points[0], points[1],
                             points[2], points[3],
                             points[4], points[5]) * matrix;
        } else if (state == Translate) {
            if (points.count() == 1)
                matrix.translate(points[0], 0);
            else if (points.count() == 2)
                matrix.translate(points[0], points[1]);
            else
                goto error;
        } else if (state == Rotate) {
            if(points.count() == 1) {
                matrix.rotate(points[0]);
            } else if (points.count() == 3) {
                matrix.translate(points[1], points[2]);
                matrix.rotate(points[0]);
                matrix.translate(-points[1], -points[2]);
            } else {
                goto error;
            }
        } else if (state == Scale) {
            if (points.count() < 1 || points.count() > 2)
                goto error;
            qreal sx = points[0];
            qreal sy = sx;
            if(points.count() == 2)
                sy = points[1];
            matrix.scale(sx, sy);
        } else if (state == SkewX) {
            if (points.count() != 1)
                goto error;
            const qreal deg2rad = qreal(0.017453292519943295769);
            matrix.shear(qTan(points[0]*deg2rad), 0);
        } else if (state == SkewY) {
            if (points.count() != 1)
                goto error;
            const qreal deg2rad = qreal(0.017453292519943295769);
            matrix.shear(0, qTan(points[0]*deg2rad));
        }
    }
error:
    return matrix;
}

utils::Transform utils::combineTransform(const Transform &parent, const QString &val)
{
    utils::Transform result = parent;
    if(!result.value.isEmpty() && !val.isEmpty())
    {
        //Separate from parent transform
        result.value.append('\n');
    }
    result.value.append(val);

    QMatrix matrix = parseTransformationMatrix(&val);
    result.matrix *= matrix;
    return result;
}
