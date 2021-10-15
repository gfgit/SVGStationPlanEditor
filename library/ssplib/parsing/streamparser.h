#ifndef SSPLIB_STREAMPARSER_H
#define SSPLIB_STREAMPARSER_H

#include <QXmlStreamReader>

namespace ssplib {

class StationPlan;

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

#endif // SSPLIB_STREAMPARSER_H
