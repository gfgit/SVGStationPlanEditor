# Library subdirectory

QT += core gui widgets svg

TEMPLATE = lib
TARGET = SVGStationPlan

CONFIG = staticlib


DEFINES += SSPLIB_ENABLE_EDITING

include(parsing/parsing.pri)
include(rendering/rendering.pri)
include(utils/utils.pri)

HEADERS += \
    $$PWD/itemtypes.h \
    $$PWD/stationplan.h \
    $$PWD/svgstationplanlib.h

SOURCES += \
    $$PWD/stationplan.cpp
