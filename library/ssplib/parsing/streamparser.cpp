#include "streamparser.h"

#include "../utils/svg_constants.h"

#include "parsinghelpers.h"

using namespace ssplib;

static const QString supportedElements[] =
    {svg_tags::RectTag,
     svg_tags::PathTag,
     svg_tags::LineTag,
     svg_tags::PolylineTag};

static bool isElementSupported(const QStringRef& tag)
{
    for(const QString& val : supportedElements)
    {
        if(val == tag)
            return true;
    }
    return false;
}

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

    return !xml.hasError();
}

void StreamParser::parseGroup()
{
    while (xml.readNextStartElement())
    {
        if (xml.name() == svg_tags::GroupTag)
            parseGroup();

        if(!isElementSupported(xml.name()))
        {
            xml.skipCurrentElement();
            continue;
        }

        utils::XmlElement e(xml.name(), xml.attributes());

        parsing::parseLabel(e, plan->labels);
        parsing::parsePlatform(e, plan->platforms);
        parsing::parseTrackConnection(e, plan->trackConnections);
    }
}
