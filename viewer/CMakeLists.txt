
#Set Win32 resources
#if (WIN32)
#    configure_file(../packaging/windows/resources.rc.in ${CMAKE_BINARY_DIR}/resources/resources.rc)
#    set(SSP_VIEWER_RESOURCES
#        ${SSP_VIEWER_RESOURCES}
#        ${CMAKE_BINARY_DIR}/resources/resources.rc
#        )
#endif()

#add_subdirectory(parsing)
#add_subdirectory(rendering)
#add_subdirectory(utils)

# Set SSPLibrary info template file
set(SSP_VIEWER_SOURCES
    ${SSP_VIEWER_SOURCES}
    mainwindow.h

    main.cpp
    mainwindow.cpp
    )

set(SSP_VIEWER_UI_FILES
    ${SSP_VIEWER_UI_FILES}
    mainwindow.ui
    )

# Create main targets
set(SSP_VIEWER_TARGET "sspviewer")

# Add executable
add_executable(${SSP_VIEWER_TARGET} WIN32
    ${SSP_VIEWER_SOURCES}
    ${SSP_VIEWER_UI_FILES}
    #${SSP_VIEWER_SOURCES}
    )

# Set compiler options
target_compile_options(
    ${SSP_VIEWER_TARGET}
    PRIVATE
    ${SSP_COMPILE_OPTIONS}
    )


# Set include directories
target_include_directories(
    ${SSP_VIEWER_TARGET}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include #For template files
    )

# Set link libraries
target_link_libraries(
    ${SSP_VIEWER_TARGET}
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    )

# Set compiler definitions
target_compile_definitions(${SSP_VIEWER_TARGET} PRIVATE ${SSP_PROJECT_DEFINITIONS})
