#include "domparser.h"

#include "../utils/svg_constants.h"

#include "parsinghelpers.h"

#ifdef SSPLIB_ENABLE_EDITING

using namespace ssplib;

DOMParser::DOMParser(QDomDocument *doc, StationPlan *ptr) :
    m_doc(doc),
    plan(ptr)
{

}

bool DOMParser::parse()
{
    return true;
}

#endif // SSPLIB_ENABLE_EDITING
