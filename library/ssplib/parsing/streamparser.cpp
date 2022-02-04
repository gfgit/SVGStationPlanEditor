#include "streamparser.h"

#include <ssplib/utils/svg_constants.h>

#include <ssplib/stationplan.h>

#include "parsinghelpers.h"

#include <QDebug>

using namespace ssplib;

StreamParser::StreamParser(StationPlan *ptr, QIODevice *dev) :
    xml(dev),
    plan(ptr)
{

}

bool StreamParser::parse()
{
    if(!xml.readNextStartElement())
    {
        //Cannot read
        return false;
    }

    if(xml.name() != QLatin1String("svg"))
    {
        //Not SVG
        return false;
    }

    parseGroup(utils::ElementStyle());

    if(xml.hasError())
    {
        qWarning() << "XML Error:" << xml.lineNumber() << xml.columnNumber() << xml.errorString();
    }

    return !xml.hasError();
}

void StreamParser::parseGroup(const utils::ElementStyle& parentStyle)
{
    utils::XmlElement groupElem(xml.name(), xml.attributes());
    utils::ElementStyle elemStyle = utils::parseStrokeWidthStyle(groupElem, parentStyle, QRectF());

    while (xml.readNextStartElement())
    {
        if(xml.name() == svg_tags::GroupTag)
        {
            parseGroup(elemStyle);
            continue;
        }
        else if(parsing::isElementSupported(xml.name()))
        {
            utils::XmlElement e(xml.name(), xml.attributes());

            parsing::parseLabel(e, plan->labels, elemStyle);
            parsing::parsePlatform(e, plan->platforms, elemStyle);
            parsing::parseTrackConnection(e, plan->trackConnections, elemStyle);
        }

        xml.skipCurrentElement();
    }
}
