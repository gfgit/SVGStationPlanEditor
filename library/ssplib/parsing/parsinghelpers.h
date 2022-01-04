#ifndef SSPLIB_PARSINGHELPERS_H
#define SSPLIB_PARSINGHELPERS_H

#include <ssplib/utils/svg_constants.h>
#include <ssplib/utils/svg_path_utils.h>
#include <ssplib/utils/xmlelement.h>

namespace ssplib {

namespace parsing {

bool parseLabel(utils::XmlElement &e, QVector<LabelItem> &labels);

bool parsePlatform(utils::XmlElement &e, QVector<TrackItem> &platforms);

bool parseTrackConnection(utils::XmlElement &e,
                          QVector<TrackConnectionItem> &connections);



bool isElementSupported(const QStringRef& tag);
bool isElementSupported(const QString& tag);

} // namespace parsing

} // namespace ssplib

#endif // SSPLIB_PARSINGHELPERS_H
