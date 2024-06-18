#ifndef SSPLIB_STATIONPLAN_H
#define SSPLIB_STATIONPLAN_H

#include "itemtypes.h"

#include <QRgb>

namespace ssplib {

class StationPlan
{
public:
    StationPlan();

    void clear();

public:
    QList<LabelItem> labels;
    QList<TrackItem> platforms;
    QList<TrackConnectionItem> trackConnections;

public:
    QString stationName;

    bool drawLabels;
    bool drawTracks;

    QRgb labelRGB;
    QRgb platformRGB;
    qreal platformPenWidth;
};

} // namespace ssplib

#endif // SSPLIB_STATIONPLAN_H
