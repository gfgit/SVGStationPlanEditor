#ifndef SSPLIB_STATIONINFOPARSER_H
#define SSPLIB_STATIONINFOPARSER_H

#include <QXmlStreamReader>

namespace ssplib {

class StationPlan;

class StationInfoReader
{
public:
    StationInfoReader(StationPlan *ptr, QIODevice *dev);

    bool parse();

private:
    void parseStation();
    void parseGate();
    void parseTrack();

private:
    QXmlStreamReader xml;
    StationPlan *m_plan;
};

class StationInfoWriter
{
public:
    StationInfoWriter(QIODevice *dev);

    bool write(const StationPlan *plan);

private:
    void writeStation(const StationPlan *plan);

private:
    QXmlStreamWriter xml;
};

} // namespace ssplib

#endif // STATIONINFOPARSER_H
