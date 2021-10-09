#ifndef STATIONPLAN_H
#define STATIONPLAN_H

#include "itemtypes.h"

#include <QRgb>

namespace ssplib {

class StationPlan
{
public:
    StationPlan();

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

#endif // STATIONPLAN_H
