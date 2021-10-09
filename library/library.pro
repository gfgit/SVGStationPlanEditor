# Library subdirectory

QT += core gui widgets svg

TEMPLATE = lib

CONFIG += staticlib

# Editing capabilities
CONFIG += editing_features
editing_features {
DEFINES += SSPLIB_ENABLE_EDITING
QT += xml
}


include(parsing/parsing.pri)
include(rendering/rendering.pri)
include(utils/utils.pri)

HEADERS += \
    $$PWD/itemtypes.h \
    $$PWD/stationplan.h \
    $$PWD/svgstationplanlib.h

SOURCES += \
    $$PWD/stationplan.cpp
