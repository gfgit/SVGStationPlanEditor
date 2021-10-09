#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include <QXmlStreamReader>

#include "../stationplan.h"

namespace ssplib {

class StreamParser
{
public:
    StreamParser(StationPlan *ptr, QIODevice *dev);

    bool parse();

private:
    void parseGroup();

private:
    QXmlStreamReader xml;
    StationPlan *plan;
};

} // namespace ssplib

#endif // STREAMPARSER_H
