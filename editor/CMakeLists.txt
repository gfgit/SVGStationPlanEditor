
#Set Win32 resources
#if (WIN32)
#    configure_file(../packaging/windows/resources.rc.in ${CMAKE_BINARY_DIR}/resources/resources.rc)
#    set(SSP_EDITOR_RESOURCES
#        ${SSP_EDITOR_RESOURCES}
#        ${CMAKE_BINARY_DIR}/resources/resources.rc
#        )
#endif()

add_subdirectory(manager)
add_subdirectory(model)
add_subdirectory(utils)
add_subdirectory(view)

# Set SSPLibrary info template file
set(SSP_EDITOR_SOURCES
    ${SSP_EDITOR_SOURCES}
    mainwindow.h

    main.cpp
    mainwindow.cpp
    )

set(SSP_EDITOR_UI_FILES
    ${SSP_EDITOR_UI_FILES}
    mainwindow.ui
    )

# Add executable
add_executable(${SSP_EDITOR_TARGET} WIN32
    ${SSP_EDITOR_SOURCES}
    ${SSP_EDITOR_UI_FILES}
    #${SSP_VIEWER_SOURCES}
    )

# Set compiler options
target_compile_options(
    ${SSP_EDITOR_TARGET}
    PRIVATE
    ${SSP_COMPILE_OPTIONS}
    )


# Set include directories
target_include_directories(
    ${SSP_EDITOR_TARGET}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include #For template files
    ${CMAKE_SOURCE_DIR}/library
    )

# Set link libraries
target_link_libraries(
    ${SSP_EDITOR_TARGET}
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::Xml
    )

target_link_libraries(
    ${SSP_EDITOR_TARGET}
    PRIVATE
    ${SSP_LIBRARY_EDIT_TARGET}
    )

# Set compiler definitions
target_compile_definitions(${SSP_EDITOR_TARGET} PRIVATE ${SSP_PROJECT_DEFINITIONS})
