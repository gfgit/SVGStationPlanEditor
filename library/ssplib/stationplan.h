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
    QVector<LabelItem> labels;
    QVector<TrackItem> platforms;
    QVector<TrackConnectionItem> trackConnections;

public:
    bool drawLabels;
    bool drawTracks;

    QRgb labelRGB;
    QRgb platforRGB;
    qreal platformPenWidth;
};

} // namespace ssplib

#endif // SSPLIB_STATIONPLAN_H
