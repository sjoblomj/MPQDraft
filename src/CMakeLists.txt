cmake_minimum_required(VERSION 3.16)
project(MPQDraftQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

# Source files
set(SOURCES
    app/gui/main_gui.cpp
    app/gui/mainwindow.cpp
    app/gui/patchwizard.cpp
    app/gui/sempqwizard.cpp
    app/gui/pluginpage.cpp
    sempq/SEMPQCreator.cpp
    core/PluginManager.cpp
    core/GameData.cpp
    core/GameDetection.cpp
    app/gui/helpers/sempqparamsbuilder.cpp
)

set(HEADERS
    app/gui/main_gui.h
    app/gui/mainwindow.h
    app/gui/patchwizard.h
    app/gui/sempqwizard.h
    app/gui/pluginpage.h
    app/gui/helpers/gamedata_qt.h
    sempq/SEMPQCreator.h
    core/PluginManager.h
    core/GameData.h
    core/GameDetection.h
    core/MPQDraftPlugin.h
    core/PatcherFlags.h
    core/PatcherApi.h
    dll/PatcherLimits.h
    app/gui/helpers/sempqparamsbuilder.h
)

# Resource files
set(RESOURCES
    app/gui/resources/mpqdraft.qrc
)

# Create executable
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
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
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/core
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
