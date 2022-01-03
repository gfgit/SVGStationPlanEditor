#include "stationinfoparser.h"

#include "ssplib/stationinfo.h"

#include "parsinghelpers.h"

#include <QDebug>

using namespace ssplib;

namespace ssp_info_tags
{
const QLatin1String XmlDocName = QLatin1String("ssp-info");
const QLatin1String Station = QLatin1String("station");
const QLatin1String GateList = QLatin1String("gate-list");
const QLatin1String Gate = QLatin1String("gate");
const QLatin1String TrackList = QLatin1String("track-list");
const QLatin1String Track = QLatin1String("track");
const QLatin1String TrackConnList = QLatin1String("track-conn-list");
}

namespace ssp_info_attrs
{
const QLatin1String Name = QLatin1String("name");
const QLatin1String Value = QLatin1String("value");
const QLatin1String TrkCount = QLatin1String("trk-count");
const QLatin1String GateSide = QLatin1String("side");
const QLatin1String TrackPos = QLatin1String("pos");
}

const QLatin1String gateSideValues[int(ssplib::Side::NSides)] = {
    QLatin1String("West"), //ssplib::Side::West
    QLatin1String("East") //ssplib::Side::East
};

StationInfoReader::StationInfoReader(StationInfo *ptr, QIODevice *dev) :
    xml(dev),
    m_info(ptr)
{

}

bool StationInfoReader::parse()
{
    if(!xml.readNextStartElement())
    {
        //Cannot read
        return false;
    }

    if(xml.name() != ssp_info_tags::XmlDocName)
    {
        //Not Station Info Xml
        return false;
    }

    while (xml.readNextStartElement())
    {
        if(xml.name() == ssp_info_tags::Station)
        {
            parseStation();
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    parseStation();

    if(xml.hasError())
    {
        qWarning() << "XML Error:" << xml.lineNumber() << xml.columnNumber() << xml.errorString();
    }

    return !xml.hasError();
}

void StationInfoReader::parseStation()
{
    m_info->stationName = xml.attributes().value(ssp_info_attrs::Name).toString();

    if(!xml.readNextStartElement())
        return;

    if(xml.name() != ssp_info_tags::GateList)
    {
        //Skip current element
        xml.skipCurrentElement();

        //Skip station
        xml.skipCurrentElement();

        return;
    }

    //Parse Gate List
    while (xml.readNextStartElement())
    {
        if(xml.name() == ssp_info_tags::Gate)
        {
            parseGate();
        }
        xml.skipCurrentElement();
    }

    if(!xml.readNextStartElement())
        return;

    if(xml.name() != ssp_info_tags::TrackList)
    {
        //Skip current element
        xml.skipCurrentElement();

        //Skip station
        xml.skipCurrentElement();

        return;
    }

    if(!xml.readNextStartElement())
        return;

    //Parse Track List
    while (xml.readNextStartElement())
    {
        if(xml.name() == ssp_info_tags::Track)
        {
            parseTrack();
        }
        xml.skipCurrentElement();
    }

    if(xml.name() != ssp_info_tags::TrackConnList)
    {
        //Skip current element
        xml.skipCurrentElement();

        //Skip station
        xml.skipCurrentElement();

        return;
    }

    //Parse track conn
    const QString conns = xml.attributes().value(ssp_info_attrs::Value).toString();
    if(!conns.isEmpty())
    {
        utils::parseTrackConnectionAttribute(conns, m_info->trackConnections);
        std::sort(m_info->trackConnections.begin(), m_info->trackConnections.end());
    }

    xml.skipCurrentElement();
}

void StationInfoReader::parseGate()
{
    StationInfo::GateInfo gate;
    QStringRef name = xml.attributes().value(ssp_info_attrs::Name).trimmed();
    if(name.isEmpty())
        return;

    gate.gateLetter = name.front().toUpper();
    if(gate.gateLetter < 'A' || gate.gateLetter > 'Z')
        return; //Invalid name

    for(const StationInfo::GateInfo& other : qAsConst(m_info->gates))
    {
        if(other.gateLetter == gate.gateLetter)
            return; //Name already exist
    }

    bool ok = false;
    gate.gateOutTrkCount = xml.attributes().value(ssp_info_attrs::TrkCount).toInt(&ok);
    if(!ok || gate.gateOutTrkCount < 0 || gate.gateOutTrkCount > 255) //TODO: max track?
        return;

    QString side = xml.attributes().value(ssp_info_attrs::GateSide).toString();
    if(side == gateSideValues[int(Side::West)])
        gate.gateSide = Side::East;
    else if(side == gateSideValues[int(Side::East)])
        gate.gateSide = Side::West;
    else
        return; //Invalid side

    m_info->gates.append(gate);
}

void StationInfoReader::parseTrack()
{
    StationInfo::TrackInfo track;
    QStringRef name = xml.attributes().value(ssp_info_attrs::Name).trimmed();
    if(name.isEmpty())
        return;

    track.trackName = name.toString();

    bool ok = false;
    track.trackPos = xml.attributes().value(ssp_info_attrs::TrackPos).toInt(&ok);
    if(!ok || track.trackPos < 0 || track.trackPos > 255) //TODO: max track?
        return;

    for(const StationInfo::TrackInfo& other : qAsConst(m_info->platforms))
    {
        if(other.trackPos == track.trackPos || other.trackName == track.trackName)
            return; //Name or Pos already exist
    }

    m_info->platforms.append(track);
}


StationInfoWriter::StationInfoWriter(QIODevice *dev) :
    xml(dev)
{

}

bool StationInfoWriter::write(StationInfo *info)
{
    xml.writeStartDocument();

    xml.writeStartElement(ssp_info_tags::XmlDocName);

    writeStation(info);

    xml.writeEndElement(); //ssp_info_tags::XmlDocName

    xml.writeEndDocument();

    return true;
}

void StationInfoWriter::writeStation(StationInfo *info)
{
    xml.writeStartElement(ssp_info_tags::Station);

    xml.writeAttribute(ssp_info_attrs::Name, info->stationName);

    xml.writeEndElement(); //ssp_info_tags::Station
}
