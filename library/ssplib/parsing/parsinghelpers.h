#ifndef SSPLIB_PARSINGHELPERS_H
#define SSPLIB_PARSINGHELPERS_H

#include "ssplib/utils/svg_constants.h"
#include "ssplib/utils/svg_path_utils.h"
#include "ssplib/utils/xmlelement.h"

namespace ssplib {

namespace parsing {

int parseLabel(utils::XmlElement &e, QVector<LabelItem> &labels);

int parsePlatform(utils::XmlElement &e, QVector<TrackItem> &platforms);

bool parseTrackConnection(utils::XmlElement &e,
                          QVector<TrackConnectionItem> &connections,
                          QVector<int> *indexes = nullptr);



bool isElementSupported(const QStringRef& tag);
bool isElementSupported(const QString& tag);

} // namespace parsing

} // namespace ssplib

#endif // SSPLIB_PARSINGHELPERS_H
