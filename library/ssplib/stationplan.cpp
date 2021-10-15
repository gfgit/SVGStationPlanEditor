#include "stationplan.h"

using namespace ssplib;

StationPlan::StationPlan() :
    drawLabels(true),
    drawTracks(true),
    labelRGB(qRgb(0, 0, 255)),
    platforRGB(qRgb(255, 0, 0)),
    platformPenWidth(10)
{

}
