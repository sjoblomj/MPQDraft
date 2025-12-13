cmake_minimum_required(VERSION 3.16)
project(MPQDraft VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate version information from current date
string(TIMESTAMP VER_MAJOR "%Y")
string(TIMESTAMP VER_MINOR "%m")
string(TIMESTAMP VER_PATCH "%d")
set(VER_BUILD 1)
set(MPQDRAFT_VERSION "${VER_MAJOR}-${VER_MINOR}-${VER_PATCH}")

# Generate version.h for CLI
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/app/cli/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/app/cli/version.h
    @ONLY
)

# Generate version_rc.h for resource files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version_rc.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/version_rc.h
    @ONLY
)

# Common include directories used by multiple targets
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/dll
    ${CMAKE_CURRENT_SOURCE_DIR}/sempq
)

# Common compile definitions
set(COMMON_DEFINITIONS
    WIN32
    _WINDOWS
    _CRT_SECURE_NO_WARNINGS
)

#############################################################################
# MPQDraftDLL - The injection DLL (Windows only)
#############################################################################
if(WIN32 OR MINGW)
    set(DLL_SOURCES
        dll/MPQDraftDLL.cpp
        dll/Patcher.cpp
        dll/PluginServer.cpp
        dll/ExceptionHandlers.cpp
        common/QDebug.cpp
        common/QHookAPI.cpp
        common/QInjectDLL.cpp
        core/GameDetection.cpp
        core/GameData.cpp
    )

    add_library(MPQDraftDLL SHARED
        ${DLL_SOURCES}
        dll/MPQDraftDLL.rc
    )

    target_include_directories(MPQDraftDLL PRIVATE ${COMMON_INCLUDE_DIRS})

    target_compile_definitions(MPQDraftDLL PRIVATE
        ${COMMON_DEFINITIONS}
        _USRDLL
        MPQDRAFTDLL_EXPORTS
    )

    target_link_libraries(MPQDraftDLL PRIVATE
        shlwapi
        psapi
    )

    # Use module definition file for exports
    # MSVC uses /DEF: syntax, MinGW uses the .def file directly
    if(MSVC)
        set_target_properties(MPQDraftDLL PROPERTIES
            LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/dll/MPQDraftDLL.def"
        )
    else()
        # For MinGW, add the .def file as a source
        target_sources(MPQDraftDLL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dll/MPQDraftDLL.def)
    endif()

    # Output to src directory for resource embedding
    set_target_properties(MPQDraftDLL PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

#############################################################################
# MPQStub - The SEMPQ stub executable (Windows only)
#############################################################################
if(WIN32 OR MINGW)
    set(STUB_SOURCES
        sempq/stub/MPQStub.cpp
        common/QDebug.cpp
        common/QInjectDLL.cpp
        common/QResource.cpp
        core/GameDetection.cpp
        core/GameData.cpp
    )

    add_executable(MPQStub WIN32
        ${STUB_SOURCES}
        sempq/stub/Stub.rc
    )

    target_include_directories(MPQStub PRIVATE ${COMMON_INCLUDE_DIRS})

    target_compile_definitions(MPQStub PRIVATE ${COMMON_DEFINITIONS})

    target_link_libraries(MPQStub PRIVATE
        shlwapi
        psapi
    )

    # Output to src directory for resource embedding
    set_target_properties(MPQStub PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # MPQStub depends on MPQDraftDLL being built first (it embeds it)
    add_dependencies(MPQStub MPQDraftDLL)
endif()

#############################################################################
# MPQDraft - The main Qt GUI application
#############################################################################

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

# Source files - Common Windows utilities
set(COMMON_SOURCES
    common/QDebug.cpp
    common/QResource.cpp
    common/QInjectDLL.cpp
)

# Resource files
set(QT_RESOURCES
    app/gui/resources/mpqdraft.qrc
)

# Determine which sources to include based on platform
if(WIN32 OR MINGW)
    # Windows: Include main entry point, CLI, common Windows utilities, and resources
    set(MAIN_SOURCES
        app/MPQDraft.cpp
        ${GUI_SOURCES}
        ${CORE_SOURCES}
        ${CLI_SOURCES}
        ${COMMON_SOURCES}
        MPQDraft.rc
    )
else()
    # Linux/other: GUI only with standalone main()
    set(MAIN_SOURCES
        ${GUI_SOURCES}
        ${CORE_SOURCES}
    )
endif()

# Create main executable
add_executable(MPQDraft WIN32
    ${MAIN_SOURCES}
    ${QT_RESOURCES}
)

# Define MPQDRAFT_INTEGRATED_BUILD on Windows so main_gui.cpp doesn't define main()
if(WIN32 OR MINGW)
    target_compile_definitions(MPQDraft PRIVATE
        MPQDRAFT_INTEGRATED_BUILD
        ${COMMON_DEFINITIONS}
    )
endif()

# Link Qt libraries
target_link_libraries(MPQDraft PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

# Windows-specific libraries
if(WIN32 OR MINGW)
    target_link_libraries(MPQDraft PRIVATE
        shlwapi
        psapi
    )
endif()

# Include directories
target_include_directories(MPQDraft PRIVATE
    ${COMMON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/app/gui/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/app/cli
)

# Compiler flags for MinGW cross-compilation
if(MINGW)
    target_compile_definitions(MPQDraft PRIVATE
        UNICODE
        _UNICODE
    )

    set_target_properties(MPQDraft PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# On Windows, MPQDraft depends on DLL and Stub being built first (they're embedded via .rc)
if(WIN32 OR MINGW)
    add_dependencies(MPQDraft MPQDraftDLL MPQStub)
endif()

# Installation
install(TARGETS MPQDraft
    RUNTIME DESTINATION bin
)

if(WIN32 OR MINGW)
    install(TARGETS MPQDraftDLL MPQStub
        RUNTIME DESTINATION bin
    )
endif()
