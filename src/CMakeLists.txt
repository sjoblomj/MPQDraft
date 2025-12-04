cmake_minimum_required(VERSION 3.16)
project(MPQDraftQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt - prefer Qt6, fall back to Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets Svg)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Svg)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
endif()

# Source files - GUI
set(GUI_SOURCES
    app/gui/main_gui.cpp
    app/gui/mainwindow.cpp
    app/gui/patchwizard.cpp
    app/gui/sempqwizard.cpp
    app/gui/pluginpage.cpp
    app/gui/helpers/sempqparamsbuilder.cpp
)

# Source files - Core (shared between GUI and CLI)
set(CORE_SOURCES
    sempq/SEMPQCreator.cpp
    core/PluginManager.cpp
    core/GameData.cpp
    core/GameDetection.cpp
)

# Source files - CLI (Windows only)
set(CLI_SOURCES
    app/cli/main_cli.cpp
    app/cli/MPQDraftCLI.cpp
    app/cli/CommandParser.cpp
)

# Source files - Common (Windows only)
set(COMMON_SOURCES
    common/QDebug.cpp
    common/QResource.cpp
    common/QInjectDLL.cpp
)

# Headers
set(HEADERS
    app/gui/main_gui.h
    app/gui/mainwindow.h
    app/gui/patchwizard.h
    app/gui/sempqwizard.h
    app/gui/pluginpage.h
    app/gui/helpers/gamedata_qt.h
    app/gui/helpers/sempqparamsbuilder.h
    sempq/SEMPQCreator.h
    core/PluginManager.h
    core/GameData.h
    core/GameDetection.h
    core/MPQDraftPlugin.h
    core/PatcherFlags.h
    core/PatcherApi.h
    dll/PatcherLimits.h
)

# Resource files
set(RESOURCES
    app/gui/resources/mpqdraft.qrc
)

# Determine which sources to include based on platform
if(WIN32)
    # Windows: Include main entry point, CLI, common Windows utilities, and resources
    set(SOURCES
        app/MPQDraft.cpp
        ${GUI_SOURCES}
        ${CORE_SOURCES}
        ${CLI_SOURCES}
        ${COMMON_SOURCES}
        MPQDraft.rc
    )
    # Define MPQDRAFT_INTEGRATED_BUILD so main_gui.cpp doesn't define main()
    add_compile_definitions(MPQDRAFT_INTEGRATED_BUILD)
else()
    # Linux/other: GUI only with standalone main()
    set(SOURCES
        ${GUI_SOURCES}
        ${CORE_SOURCES}
    )
endif()

# Create executable
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

# Windows-specific libraries
if(WIN32 OR MINGW)
    target_link_libraries(${PROJECT_NAME}
        shlwapi
        psapi
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/app/cli
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/sempq
)

# Compiler flags for MinGW cross-compilation
if(MINGW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
    )

    # Enable Windows subsystem
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
