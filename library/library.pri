# Library subdirectory

DEFINES += SSPLIB_ENABLE_EDITING

include(parsing/parsing.pri)
include(rendering/rendering.pri)
include(utils/utils.pri)

HEADERS += \
    $$PWD/itemtypes.h \
    $$PWD/stationplan.h

SOURCES += \
    $$PWD/stationplan.cpp
