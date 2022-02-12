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

# Include our custom FindXXX moudules in '/cmake' subdirectory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # Fix for emscripten not locating Qt5
    # It doesn't detect Qt from just Qt5_DIR, instead it wants all Qt5{module}_DIR set
    # Set MY_Qt5_CMAKE_DIR to your Qt5 cmake directory (e.g. /path/to/qt/5.15.2/wasm_32/lib/cmake)
    if(NOT MY_Qt5_CMAKE_DIR)
        set(MY_Qt5_CMAKE_DIR /home/filippo/Qt/5.15.2/wasm_32/lib/cmake)
    endif()

    file(GLOB files "${MY_Qt5_CMAKE_DIR}/*")
    foreach(file ${files})
        string(FIND ${file} "/" LAST_SLASH_POS REVERSE)
        MATH(EXPR LAST_SLASH_POS "${LAST_SLASH_POS}+1")
        string(SUBSTRING ${file} ${LAST_SLASH_POS} -1 MY_QT_DIR_NAME)

        set(${MY_QT_DIR_NAME}_DIR ${file})
    endforeach()
endif()

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

include(LocateWinDeployQt)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    include(QtWasmCMake)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Create main targets
set(SSP_LIBRARY_TARGET "ssplib")
set(SSP_LIBRARY_EDIT_TARGET "sspeditlib")
set(SSP_EDITOR_TARGET "sspeditor")
set(SSP_VIEWER_TARGET "sspviewer")

add_subdirectory(editor)
add_subdirectory(library)
add_subdirectory(viewer)

## Install and Deploy ##

if(WIN32)
    if(RUN_WINDEPLOYQT)
        if(NOT TARGET windeployqt_exe)
            message(FATAL_ERROR "In order to run windeployqt you must first set the exe path in WINDEPLOYQT_EXE_DIR")
        endif()

        install(CODE "
            message(STATUS \"Running windeployqt ${windeployqt_exe}\")
            execute_process(COMMAND ${windeployqt_exe} ${CMAKE_INSTALL_PREFIX}/bin
                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
                OUTPUT_VARIABLE WINDEPLOYQT_EXE_RESULT
                ERROR_VARIABLE WINDEPLOYQT_EXE_RESULT)

            message(STATUS \${WINDEPLOYQT_EXE_RESULT})

            message(STATUS \"${windeployqt_exe} Done.\")
            ")
    endif()
endif()

## Install end ##
