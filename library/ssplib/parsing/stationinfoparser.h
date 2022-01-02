#ifndef SSPLIB_STATIONINFOPARSER_H
#define SSPLIB_STATIONINFOPARSER_H

#include <QXmlStreamReader>

namespace ssplib {

class StationInfo;

class StationInfoReader
{
public:
    StationInfoReader(StationInfo *ptr, QIODevice *dev);

    bool parse();

private:
    void parseStation();
    void parseGate();
    void parseTrack();

private:
    QXmlStreamReader xml;
    StationInfo *m_info;
};

} // namespace ssplib

#endif // STATIONINFOPARSER_H
