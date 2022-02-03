#ifndef SSPLIB_DOMPARSER_H
#define SSPLIB_DOMPARSER_H

#ifdef SSPLIB_ENABLE_EDITING

class QDomDocument;
class QDomElement;
class QString;

namespace ssplib {

class StationPlan;
class EditingInfo;

class DOMParser
{
public:
    DOMParser(QDomDocument *doc, StationPlan *ptr, EditingInfo *info);

    bool parse();

private:
    void processGroup(QDomElement& g);
    void processDefs(QDomElement& defs);
    void processText(QDomElement& text);
    void processTspan(QDomElement &tspan, QDomElement &text);
    void processInternalTspan(QDomElement &top, QDomElement &cur, QString &value);

private:
    QDomDocument *m_doc;
    StationPlan *plan;
    EditingInfo *m_info;
};

} // namespace ssplib

#endif // SSPLIB_ENABLE_EDITING

#endif // SSPLIB_DOMPARSER_H
