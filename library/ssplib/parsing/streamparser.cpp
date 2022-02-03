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

    parseGroup();

    if(xml.hasError())
    {
        qWarning() << "XML Error:" << xml.lineNumber() << xml.columnNumber() << xml.errorString();
    }

    return !xml.hasError();
}

void StreamParser::parseGroup()
{
    while (xml.readNextStartElement())
    {
        if(xml.name() == svg_tags::GroupTag)
        {
            parseGroup();
            continue;
        }
        else if(parsing::isElementSupported(xml.name()))
        {
            utils::XmlElement e(xml.name(), xml.attributes());

            parsing::parseLabel(e, plan->labels);
            parsing::parsePlatform(e, plan->platforms);
            parsing::parseTrackConnection(e, plan->trackConnections);
        }

        xml.skipCurrentElement();
    }
}
