#ifndef SSPLIB_PARSINGHELPERS_H
#define SSPLIB_PARSINGHELPERS_H

#include "../utils/svg_constants.h"
#include "../utils/svg_path_utils.h"
#include "../utils/xmlelement.h"

namespace ssplib {

namespace parsing {

int parseLabel(utils::XmlElement &e, QVector<LabelItem> &labels);

int parsePlatform(utils::XmlElement &e, QVector<TrackItem> &platforms);

bool parseTrackConnection(utils::XmlElement &e,
                          QVector<TrackConnectionItem> &connections,
                          QVector<int> *indexes = nullptr);

} // namespace parsing

} // namespace ssplib

#endif // SSPLIB_PARSINGHELPERS_H
