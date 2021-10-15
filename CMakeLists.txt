cmake_minimum_required(VERSION 3.17)
include(CMakeDependentOption)

project(SVGStationPlanEditor VERSION 1.0.0 LANGUAGES CXX)

option(UPDATE_TS "Update translations" OFF)
option(UPDATE_TS_KEEP_OBSOLETE "Keep obsolete entries when updating translations" ON)
option(BUILD_DOXYGEN "Build Doxygen documentation" OFF)

if (WIN32)
    option(RUN_WINDEPLOYQT "Run windeployqt after executable is installed" ON)
endif()

## Defines ##

set(APP_PRODUCT_NAME "SVGStationPlanEditor")
set(APP_DISPLAY_NAME "SVG StationPlan Editor")
set(APP_COMPANY_NAME "Train Software")

set(PROJECT_HOMEPAGE_URL "https://github.com/gfgit/SVGStationPlanEditor")
set(APP_HELP_URL ${PROJECT_HOMEPAGE_URL})
set(APP_UPDATE_URL ${PROJECT_HOMEPAGE_URL})

set(PROJECT_DESCRIPTION "${APP_DISPLAY_NAME} lets you create station plan SVGs")

#set(APP_ICON ${CMAKE_SOURCE_DIR}/files/icons/icon.ico)

## defines end ##

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SSP_PROJECT_DEFINITIONS ${SSP_PROJECT_DEFINITIONS} -DAPPVERSION="${PROJECT_VERSION}" -DQT_DEPRECATED_WARNINGS)

find_package(Qt5 REQUIRED
    COMPONENTS
    Core
    Gui
    Widgets
    Svg
    LinguistTools)

find_package(Qt5
    COMPONENTS
    Xml)

if(BUILD_DOXYGEN)
    find_package(Doxygen)
endif()

if(MSVC)
    set(SSP_COMPILE_OPTIONS
        /WX
        /wd4267
        /wd4244
        /experimental:external
        /external:anglebrackets
        /external:W0
        "$<$<CONFIG:RELEASE>:/O2>"
        "$<$<COMPILE_LANGUAGE:CXX>:/MP>")
else()
    set(SSP_COMPILE_OPTIONS
        "$<$<CONFIG:RELEASE>:-O2>"
        #-Werror
        -Wuninitialized
        -pedantic-errors
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wshadow)
endif()


#Locate windeployqt
if(WIN32 AND RUN_WINDEPLOYQT AND NOT WINDEPLOYQT_EXE)
    set(WINDEPLOYQT_EXE_TMP NOTFOUND)
    message("Searching windeployqt executable")
    if(QT_QMAKE_EXECUTABLE)
        get_filename_component(WINDEPLOYQT_DIR ${QT_QMAKE_EXECUTABLE} DIRECTORY)
        set(WINDEPLOYQT_EXE_TMP "${WINDEPLOYQT_DIR}/windeployqt.exe")
    endif()
    if(NOT EXISTS ${WINDEPLOYQT_EXE_TMP} AND Qt5_DIR)
        get_filename_component(WINDEPLOYQT_EXE_TMP "${Qt5_DIR}/../../../bin/windeployqt.exe" REALPATH)
    endif()

    if(EXISTS ${WINDEPLOYQT_EXE_TMP})
        message("Found ${WINDEPLOYQT_EXE_TMP}")
    else()
        message("windeployqt NOT FOUND")
        set(WINDEPLOYQT_EXE_TMP NOTFOUND)
    endif()
    set(WINDEPLOYQT_EXE ${WINDEPLOYQT_EXE_TMP} CACHE FILEPATH "windeployqt executable file path.")
    unset(WINDEPLOYQT_EXE_TMP)
    unset(WINDEPLOYQT_DIR)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(editor)
add_subdirectory(library)
add_subdirectory(viewer)
