
#Set Win32 resources
#if (WIN32)
#    configure_file(../packaging/windows/resources.rc.in ${CMAKE_BINARY_DIR}/resources/resources.rc)
#    set(SSP_LIBRARY_RESOURCES
#        ${SSP_LIBRARY_RESOURCES}
#        ${CMAKE_BINARY_DIR}/resources/resources.rc
#        )
#endif()

add_subdirectory(parsing)
add_subdirectory(rendering)
add_subdirectory(utils)

# Set SSPLibrary info template file

set(SSP_LIBRARY_HEADERS
    ${SSP_LIBRARY_HEADERS}
    itemtypes.h
    stationplan.h
    svgstationplanlib.h
    )

set(SSP_LIBRARY_SOURCES
    ${SSP_LIBRARY_SOURCES}
    stationplan.cpp
    )

# Add library
add_library(${SSP_LIBRARY_TARGET} STATIC
    ${SSP_LIBRARY_SOURCES}
    ${SSP_LIBRARY_HEADERS}
    #${SSP_LIBRARY_RESOURCES}
    )

add_library(${SSP_LIBRARY_EDIT_TARGET} STATIC
    ${SSP_LIBRARY_SOURCES}
    ${SSP_LIBRARY_HEADERS}
    #${SSP_LIBRARY_RESOURCES}
    )

# Set compiler options
target_compile_options(
    ${SSP_LIBRARY_TARGET}
    PRIVATE
    ${SSP_COMPILE_OPTIONS}
    )
target_compile_options(
    ${SSP_LIBRARY_EDIT_TARGET}
    PRIVATE
    ${SSP_COMPILE_OPTIONS}
    )


# Set include directories
target_include_directories(
    ${SSP_LIBRARY_TARGET}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include #For template files
    )

target_include_directories(
    ${SSP_LIBRARY_EDIT_TARGET}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include #For template files
    )

# Set link libraries
target_link_libraries(
    ${SSP_LIBRARY_TARGET}
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    )

target_link_libraries(
    ${SSP_LIBRARY_EDIT_TARGET}
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::Xml
    )

# Set compiler definitions
target_compile_definitions(${SSP_LIBRARY_TARGET} PRIVATE ${SSP_PROJECT_DEFINITIONS})
target_compile_definitions(${SSP_LIBRARY_EDIT_TARGET} PRIVATE ${SSP_PROJECT_DEFINITIONS} -DSSPLIB_ENABLE_EDITING)
