#ifndef SSPLIB_PARSINGHELPERS_H
#define SSPLIB_PARSINGHELPERS_H

#include <ssplib/utils/svg_constants.h>
#include <ssplib/utils/svg_path_utils.h>
#include <ssplib/utils/xmlelement.h>

namespace ssplib {

namespace parsing {

bool parseLabel(utils::XmlElement &e, QList<LabelItem> &labels, const utils::ElementStyle &parentStyle);

bool parsePlatform(utils::XmlElement &e, QList<TrackItem> &platforms, const utils::ElementStyle &parentStyle);

bool parseTrackConnection(utils::XmlElement &e,
                          QList<TrackConnectionItem> &connections,
                          const utils::ElementStyle &parentStyle);



bool isElementSupported(const QStringView &tag);
bool isElementSupported(const QString& tag);

} // namespace parsing

} // namespace ssplib

#endif // SSPLIB_PARSINGHELPERS_H
