#ifndef NODEFINDERUTILS_H
#define NODEFINDERUTILS_H

#include <QString>
#include <QDomElement>
#include <QPainterPath>

struct ElementPath
{
    QDomElement elem;
    QPainterPath path;
};

typedef struct ItemBase
{
    QVector<ElementPath> elements;
    bool visible;
} ItemBase;

typedef struct LabelItem : ItemBase
{
    QChar gateLetter;
} LabelItem;

typedef struct TrackItem : ItemBase
{
    QString trackName;
    int trackPos;
} TrackItem;

//SVG attribute names
namespace svg_attr {

const QString LabelName = QLatin1String("labelname");
const QString TrackPos = QLatin1String("trackpos");

}

#endif // NODEFINDERUTILS_H
