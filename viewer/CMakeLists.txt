
#Set Win32 resources
#if (WIN32)
#    configure_file(../packaging/windows/resources.rc.in ${CMAKE_BINARY_DIR}/resources/resources.rc)
#    set(SSP_VIEWER_RESOURCES
#        ${SSP_VIEWER_RESOURCES}
#        ${CMAKE_BINARY_DIR}/resources/resources.rc
#        )
#endif()

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
    ${CMAKE_SOURCE_DIR}/library
    )

# Set link libraries
target_link_libraries(
    ${SSP_VIEWER_TARGET}
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    )

target_link_libraries(
    ${SSP_VIEWER_TARGET}
    PRIVATE
    ${SSP_LIBRARY_TARGET}
    )

# Set compiler definitions
target_compile_definitions(${SSP_VIEWER_TARGET} PRIVATE ${SSP_PROJECT_DEFINITIONS})

## Install and Deploy ##

# Copy executable
install(TARGETS ${SSP_VIEWER_TARGET}
    RUNTIME
    DESTINATION bin)

## Install end ##
