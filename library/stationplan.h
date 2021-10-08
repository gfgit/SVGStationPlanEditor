#ifndef STATIONPLAN_H
#define STATIONPLAN_H

#include "itemtypes.h"

namespace ssplib {

class StationPlan
{
public:
    StationPlan();

public:
    QVector<LabelItem> labels;
    QVector<TrackItem> platforms;
    QVector<TrackConnectionItem> trackConnections;
};

} // namespace ssplib

#endif // STATIONPLAN_H
