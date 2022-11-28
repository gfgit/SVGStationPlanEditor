#include "svgcreatorsvgwriter.h"

#include "svg_creator/svgcreatormanager.h"

#include <utils/svg_constants.h>
#include <utils/svg_trackconn_util.h>

#include <QXmlStreamWriter>

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QFont>

inline QLineF mapLineToScene(QGraphicsItem *item, const QLineF& l)
{
    return QLineF(item->mapToScene(l.p1()),
                  item->mapToScene(l.p2()));
}

inline void writeLine(QXmlStreamWriter &xml, const QLineF& l, const QPen& pen)
{
    xml.writeAttribute(QLatin1String("x1"), QString::number(l.x1()));
    xml.writeAttribute(QLatin1String("y1"), QString::number(l.y1()));
    xml.writeAttribute(QLatin1String("x2"), QString::number(l.x2()));
    xml.writeAttribute(QLatin1String("y2"), QString::number(l.y2()));
    xml.writeAttribute(QLatin1String("stroke"), pen.color().name(QColor::HexRgb));
    xml.writeAttribute(QLatin1String("stroke-width"),
                       QString::number(pen.widthF()) + QLatin1String("pt"));
    xml.writeAttribute(QLatin1String("stroke-linecap"), QLatin1String("round"));
}

inline void writeRect(QXmlStreamWriter &xml, const QRectF& r, const QBrush& br)
{
    xml.writeAttribute(QLatin1String("x"), QString::number(r.x()));
    xml.writeAttribute(QLatin1String("y"), QString::number(r.y()));
    xml.writeAttribute(QLatin1String("width"), QString::number(r.width()));
    xml.writeAttribute(QLatin1String("height"), QString::number(r.height()));
    xml.writeAttribute(QLatin1String("fill"), br.color().name(QColor::HexRgb));
}

void writeTextItemAttrs(QXmlStreamWriter &xml, QGraphicsSimpleTextItem *item)
{
    xml.writeStartElement(QLatin1String("text"));

    QPointF pos = item->scenePos();

    xml.writeAttribute(QLatin1String("x"), QString::number(pos.x()));
    xml.writeAttribute(QLatin1String("y"), QString::number(pos.y()));

    QFont font = item->font();
    QString style = QLatin1String("font-size:");
    if(font.pixelSize() != -1)
    {
        style += QString::number(font.pixelSize());
        style += QLatin1String("px");
    }else{
        style += QString::number(font.pointSizeF());
        style += QLatin1String("pt");
    }
    style += QLatin1String(";");

    int svgWeight = font.weight();
    switch (svgWeight)
    {
    case QFont::Light:
        svgWeight = 100;
        break;
    case QFont::Normal:
        svgWeight = 400;
        break;
    case QFont::Bold:
        svgWeight = 700;
        break;
    default:
        svgWeight *= 10;
    }
    style += QLatin1String("font-family:") + font.family() + ';';
    style += QLatin1String("font-weight:") + QString::number(svgWeight) + ';';
    style += QLatin1String("font-style:%1;").arg(font.italic() ? QLatin1String("italic") : QLatin1String("normal"));

    style += QLatin1String("text-align:center;");

    if(style.endsWith(';'))
        style.chop(1);
    xml.writeAttribute(QLatin1String("style"), style);

    xml.writeCharacters(item->text());

    xml.writeEndElement();
}

SvgCreatorSVGWriter::SvgCreatorSVGWriter(SvgCreatorManager *mgr) :
    manager(mgr)
{

}

bool SvgCreatorSVGWriter::writeSVG(QIODevice *dev)
{
    QXmlStreamWriter xml(dev);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();

    QRectF rect = manager->getScene()->sceneRect();
    const QString lengthFmt = QStringLiteral("%1pt");
    const QString viewBoxStr = QStringLiteral("%1 %2 %3 %4")
        .arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());

    //SVG element
    xml.writeStartElement(QLatin1String("svg"));
    xml.writeDefaultNamespace(QLatin1String("http://www.w3.org/2000/svg"));
    xml.writeAttribute(QLatin1String("version"), QLatin1String("1.1"));
    xml.writeAttribute(QLatin1String("width"), lengthFmt.arg(rect.width()));
    xml.writeAttribute(QLatin1String("height"), lengthFmt.arg(rect.height()));
    xml.writeAttribute(QLatin1String("viewBox"), viewBoxStr);

    //Write track connections
    for(const auto& conn : qAsConst(manager->trackConnections))
    {
        writeTrackConn(xml, conn);
    }

    //Write platforms
    for(const auto& platf : qAsConst(manager->platforms))
    {
        writePlatform(xml, platf);
    }

    //Write Gates
    for(const auto& gate : qAsConst(manager->gates))
    {
        writeGate(xml, gate);
    }

    //Station Label
    writeStLabel(xml);

    xml.writeEndElement(); // </svg>

    xml.writeEndDocument();
    return true;
}

void SvgCreatorSVGWriter::writeTrackConn(QXmlStreamWriter &xml, const TrackConnectionItem *item)
{
    QLineF line = mapLineToScene(item->lineItem, item->lineItem->line());

    xml.writeStartElement(QLatin1String("line"));
    writeLine(xml, line, item->lineItem->pen());

    //Add track connections
    QString connAttr = ssplib::utils::trackConnInfoToString(item->connections);
    xml.writeAttribute(ssplib::svg_attr::TrackConnections, connAttr);

    xml.writeEndElement();
}

void SvgCreatorSVGWriter::writePlatform(QXmlStreamWriter &xml, const PlatformItem *item)
{
    QLineF line = mapLineToScene(item->lineItem, item->lineItem->line());

    xml.writeStartElement(QLatin1String("g"));

    xml.writeStartElement(QLatin1String("line"));
    writeLine(xml, line, item->lineItem->pen());

    //Add platform info
    xml.writeAttribute(ssplib::svg_attr::TrackPos, QString::number(item->platfNum));

    xml.writeEndElement(); // </line>

    //Add label background
    xml.writeStartElement("rect");
    writeRect(xml, item->nameBgRect->sceneBoundingRect(), item->nameBgRect->brush());
    xml.writeEndElement(); // </rect>

    //Add label text
    writeTextItemAttrs(xml, item->nameText);

    xml.writeEndElement(); // </g>
}

void SvgCreatorSVGWriter::writeGate(QXmlStreamWriter &xml, const GateItem *item)
{
    xml.writeStartElement(QLatin1String("g"));

    //Gate tracks
    for(const auto& track : item->outTracks)
    {
        QLineF line = mapLineToScene(track.trackLineItem, track.trackLineItem->line());

        xml.writeStartElement(QLatin1String("line"));
        writeLine(xml, line, track.trackLineItem->pen());

        //Add gate track info
        xml.writeAttribute(ssplib::svg_attr::GateTrackNum, QString::number(track.number));
        xml.writeAttribute(ssplib::svg_attr::LabelName, item->gateLetter);

        xml.writeEndElement(); // </line>

        track.trackLabelItem->text();

        //Add track label
        writeTextItemAttrs(xml, track.trackLabelItem);
    }

    //Add Gate Rect
    xml.writeStartElement("rect");
    writeRect(xml,
              item->gateStationRect->mapRectToScene(item->gateStationRect->rect()),
              item->gateStationRect->brush());

    //Add gate track info
    xml.writeAttribute(ssplib::svg_attr::LabelName, item->gateLetter);
    xml.writeEndElement(); // </rect>

    //Add Gate label
    writeTextItemAttrs(xml, item->gateLabel);

    xml.writeEndElement(); // </g>
}

void SvgCreatorSVGWriter::writeStLabel(QXmlStreamWriter &xml)
{
    xml.writeStartElement(QLatin1String("g"));

    //Background rect
    xml.writeStartElement(QLatin1String("rect"));
    writeRect(xml,
              manager->stLabel.bgRect->mapRectToScene(manager->stLabel.bgRect->rect()),
              manager->stLabel.bgRect->brush());
    xml.writeEndElement(); // </rect>

    //Text
    writeTextItemAttrs(xml, manager->stLabel.text);

    xml.writeEndElement(); // </g>
}
