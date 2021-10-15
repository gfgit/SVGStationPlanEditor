
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
    ${CMAKE_SOURCE_DIR}/library
    )

target_include_directories(
    ${SSP_LIBRARY_EDIT_TARGET}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/library
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

# Install

MACRO(INSTALL_HEADERS_WITH_DIRECTORY HEADER_LIST PREFIX)

    FOREACH(HEADER ${${HEADER_LIST}})
        file(REAL_PATH ${HEADER} TMP_PATH BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        get_filename_component(DIR ${TMP_PATH} DIRECTORY)
        file(RELATIVE_PATH TMP_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${DIR})
        INSTALL(FILES ${HEADER} DESTINATION ${PREFIX}${TMP_PATH})
    ENDFOREACH(HEADER)

ENDMACRO(INSTALL_HEADERS_WITH_DIRECTORY)

# Export include path
target_include_directories(${SSP_LIBRARY_TARGET}
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>"
)
target_include_directories(${SSP_LIBRARY_EDIT_TARGET}
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           "$<INSTALL_INTERFACE:include>"
)

INSTALL_HEADERS_WITH_DIRECTORY(SSP_LIBRARY_HEADERS "include/ssplib/")

install(TARGETS ${SSP_LIBRARY_TARGET}
    EXPORT ${SSP_LIBRARY_TARGET}
    )

install(TARGETS ${SSP_LIBRARY_EDIT_TARGET}
    EXPORT ${SSP_LIBRARY_TARGET}
    )

install(EXPORT ${SSP_LIBRARY_TARGET}
    NAMESPACE "ssplib::"
    DESTINATION "lib/cmake/ssplib"
    FILE "ssplib-config.cmake")
