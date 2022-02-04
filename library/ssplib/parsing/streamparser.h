#ifndef SSPLIB_STREAMPARSER_H
#define SSPLIB_STREAMPARSER_H

#include <QXmlStreamReader>

namespace ssplib {

namespace utils {
struct ElementStyle;
}

class StationPlan;

class StreamParser
{
public:
    StreamParser(StationPlan *ptr, QIODevice *dev);

    bool parse();

private:
    void parseGroup(const ssplib::utils::ElementStyle &parentStyle);

private:
    QXmlStreamReader xml;
    StationPlan *plan;
};

} // namespace ssplib

#endif // SSPLIB_STREAMPARSER_H
