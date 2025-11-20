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
    src/main.cpp
    src/mainwindow.cpp
    src/patchwizard.cpp
    src/sempqwizard.cpp
    src/pluginpage.cpp
    src/common/pluginloader.cpp
    src/common/gamedata.cpp
    src/common/gamedetection.cpp
)

set(HEADERS
    src/mainwindow.h
    src/patchwizard.h
    src/sempqwizard.h
    src/pluginpage.h
    src/gamedata_qt.h
    src/common/pluginloader.h
    src/common/mpqdraftplugin.h
    src/common/common.h
    src/common/patcher.h
    src/common/gamedata.h
    src/common/gamedetection.h
)

# Resource files
set(RESOURCES
    resources/mpqdraft.qrc
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
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

