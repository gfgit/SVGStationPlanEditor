#ifndef SSPLIB_STATIONINFO_H
#define SSPLIB_STATIONINFO_H

#include "itemtypes.h"

#include <QRgb>

namespace ssplib {

class StationInfo
{
public:
    struct GateInfo
    {
        QChar gateLetter;
        int   gateOutTrkCount = 0;
        Side  gateSide = Side::NSides;
    };

    struct TrackInfo
    {
        QString trackName;
        int trackPos = 0;
    };

    StationInfo();

    void clear();

public:
    QString stationName;

    QVector<GateInfo> gates;
    QVector<TrackInfo> platforms;
    QVector<TrackConnectionInfo> trackConnections;
};

} // namespace ssplib

#endif // SSPLIB_STATIONINFO_H
