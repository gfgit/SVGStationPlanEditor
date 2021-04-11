QT       += core gui svg xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

LIBS += C:\Windows\System32\ws2_32.dll

#libpqxx
INCLUDEPATH += C:/Filippo/Qt_Project/libraries/build-libpqxx-7.4.1-Desktop_Qt_5_15_2_MinGW_64_bit-Release/include
INCLUDEPATH += C:/Filippo/Qt_Project/libraries/libpqxx-7.4.1/include
LIBS += C:/Filippo/Qt_Project/libraries/build-libpqxx-7.4.1-Desktop_Qt_5_15_2_MinGW_64_bit-Release/src/libpqxx.a

#libpq
LIBS += C:/PROGRA~1/PostgreSQL/13/lib/libpq.dll

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connectionmanager.cpp \
    customsvgnodefinder.cpp \
    customsvgwidget.cpp \
    elementclass.cpp \
    main.cpp \
    mainwindow.cpp \
    nodefinderdockwidget.cpp \
    nodefinderelementclass.cpp \
    nodefinderelementwalker.cpp \
    nodefinderlabelmodel.cpp \
    nodefindermgr.cpp \
    nodefinderstationtracksmodel.cpp \
    nodefinderstatuswidget.cpp \
    nodefindersvgconverter.cpp \
    nodefindersvgwidget.cpp \
    svgtinyconverter.cpp \
    svgutils.cpp

HEADERS += \
    connectionmanager.h \
    customsvgnodefinder.h \
    customsvgwidget.h \
    elementclass.h \
    mainwindow.h \
    nodefinderdockwidget.h \
    nodefinderelementclass.h \
    nodefinderelementwalker.h \
    nodefinderlabelmodel.h \
    nodefindermgr.h \
    nodefinderstationtracksmodel.h \
    nodefinderstatuswidget.h \
    nodefindersvgconverter.h \
    nodefindersvgwidget.h \
    svgtinyconverter.h \
    svgutils.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
