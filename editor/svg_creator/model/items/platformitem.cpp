#include "platformitem.h"

#include <QGraphicsLineItem>
#include <QGraphicsScene>

#include <QXmlStreamWriter>

const QLatin1String SvgNameProp("platf_name");
const QLatin1String SvgNumberProp("platf_num");

const QLatin1String NameProp("Name");
const QLatin1String NumberProp("Number");
const QLatin1String x1Prop("x1");
const QLatin1String y1Prop("y1");
const QLatin1String x2Prop("x2");
const QLatin1String y2Prop("y2");

void insertLine(QVariantMap& map, const QLineF& l, const QLatin1String& prefix = QLatin1String())
{
    map.insert(prefix + x1Prop, l.x1());
    map.insert(prefix + y1Prop, l.y1());
    map.insert(prefix + x2Prop, l.x2());
    map.insert(prefix + y2Prop, l.y2());
}

QLineF extractLine(const QVariantMap& map, const QLatin1String& prefix = QLatin1String())
{
    QPointF a(map.value(prefix + x1Prop).toDouble(), map.value(prefix + y1Prop).toDouble());
    QPointF b(map.value(prefix + x2Prop).toDouble(), map.value(prefix + y2Prop).toDouble());
    return QLineF(a, b);
}

void writeSVGLine(QXmlStreamWriter &xml, const QLineF& l, const QXmlStreamAttributes& attrs)
{
    xml.writeStartElement(QLatin1String("line"));
    xml.writeAttribute(x1Prop, QString::number(l.x1()));
    xml.writeAttribute(y1Prop, QString::number(l.y1()));
    xml.writeAttribute(x2Prop, QString::number(l.x2()));
    xml.writeAttribute(y2Prop, QString::number(l.y2()));
    xml.writeAttribute(QLatin1String("style"), QLatin1String("stroke:rgb(0,0,0);stroke-width:2"));
    xml.writeAttributes(attrs);
    xml.writeEndElement();
}

PlatformLayoutItem::PlatformLayoutItem()
{
    platfLineItem = new QGraphicsLineItem;
}

QString PlatformLayoutItem::itemType()
{
    return QStringLiteral("Platform");
}

QVector<AbstractLayoutItem::PropertyDescr> PlatformLayoutItem::getPropertyDescriptors()
{
    QVector<PropertyDescr> vec;

    vec.append(PropertyDescr{NameProp,
                             QLatin1String("Platform name"),
                             QVariant::String});

    vec.append(PropertyDescr{NumberProp,
                             QLatin1String("Platform ordered position"),
                             QVariant::Int});

    vec.append(PropertyDescr{x1Prop,
                             QLatin1String("Item left X coordinate"),
                             QVariant::Double});

    vec.append(PropertyDescr{y1Prop,
                             QLatin1String("Item left Y coordinate"),
                             QVariant::Double});

    vec.append(PropertyDescr{x2Prop,
                             QLatin1String("Item right X coordinate"),
                             QVariant::Double});

    vec.append(PropertyDescr{y2Prop,
                             QLatin1String("Item right Y coordinate"),
                             QVariant::Double});

    return vec;
}

QVariantMap PlatformLayoutItem::getProperties()
{
    QVariantMap map;
    map.insert(NameProp, platfName);
    map.insert(NumberProp, platfName);
    insertLine(map, platfLineItem->line());
    return map;
}

bool PlatformLayoutItem::setProperties(const QVariantMap &map)
{
    platfName = map.value(NameProp).toString();
    platfNum = map.value(NumberProp).toInt();
    platfLineItem->setLine(extractLine(map));
    return true;
}

bool PlatformLayoutItem::addToScene(QGraphicsScene *scene)
{
    scene->addItem(platfLineItem);
    return true;
}

void PlatformLayoutItem::writeSVGElements(QXmlStreamWriter &xml)
{
    QXmlStreamAttributes attrs;
    attrs.append(SvgNameProp, platfName);
    attrs.append(SvgNumberProp, QString::number(platfNum));
    writeSVGLine(xml, platfLineItem->line(), attrs);
}
