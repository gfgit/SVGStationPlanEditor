#include "stationinfo.h"

using namespace ssplib;

StationInfo::StationInfo()
{

}

void StationInfo::clear()
{
    stationName.clear();
    stationName.squeeze();

    gates.clear();
    gates.squeeze();

    platforms.clear();
    platforms.squeeze();

    trackConnections.clear();
    trackConnections.squeeze();
}
