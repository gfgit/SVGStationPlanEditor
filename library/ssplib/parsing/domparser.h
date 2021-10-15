#ifndef SSPLIB_DOMPARSER_H
#define SSPLIB_DOMPARSER_H

#ifdef SSPLIB_ENABLE_EDITING

#include "../stationplan.h"

class QDomDocument;

namespace ssplib {

class DOMParser
{
public:
    DOMParser(QDomDocument *doc, StationPlan *ptr);

    bool parse();

private:
    QDomDocument *m_doc;
    StationPlan *plan;
};

} // namespace ssplib

#endif // SSPLIB_ENABLE_EDITING

#endif // SSPLIB_DOMPARSER_H
