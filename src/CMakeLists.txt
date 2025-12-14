cmake_minimum_required(VERSION 3.16)
project(MPQDraft VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate version information from current date
string(TIMESTAMP VER_MAJOR "%Y")
string(TIMESTAMP VER_MINOR "%m")
string(TIMESTAMP VER_PATCH "%d")
set(VER_BUILD 1)

# Filename version keeps zero-padding (e.g., "2025-12-09")
set(MPQDRAFT_VERSION "${VER_MAJOR}-${VER_MINOR}-${VER_PATCH}")

# Strip leading zeros for C/C++ macros to avoid octal interpretation (e.g., "09" -> "9")
math(EXPR VER_MINOR "${VER_MINOR}")
math(EXPR VER_PATCH "${VER_PATCH}")

configure_file(app/cli/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/app/cli/version.h @ONLY)
configure_file(version_rc.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version_rc.h @ONLY)

# Common include directories used by multiple targets
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/dll
    ${CMAKE_CURRENT_SOURCE_DIR}/sempq
)

# Common compile definitions
# Note: We explicitly undefine UNICODE and _UNICODE to use ANSI versions of
# Windows API functions (char* instead of wchar_t*). MXE enables UNICODE by
# default, which would break the existing codebase.
set(COMMON_DEFINITIONS WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _MBCS)

# Ensure UNICODE is not defined (use ANSI APIs)
if(MINGW)
    add_compile_options(-UUNICODE -U_UNICODE)
endif()

# Helper function to configure Windows targets with common settings
function(configure_windows_target target)
    target_include_directories(${target} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE ${COMMON_DEFINITIONS})
    target_link_libraries(${target} PRIVATE shlwapi psapi)
    if(MSVC)
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endfunction()

#############################################################################
# Windows-only targets: MPQDraftDLL and MPQStub
#############################################################################
if(WIN32 OR MINGW)
    # MPQDraftDLL - The injection DLL
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
    if(MSVC)
        list(APPEND DLL_SOURCES dll/MPQDraftDLL.rc)
    endif()

    add_library(MPQDraftDLL SHARED ${DLL_SOURCES})
    configure_windows_target(MPQDraftDLL)
    target_compile_definitions(MPQDraftDLL PRIVATE _USRDLL MPQDRAFTDLL_EXPORTS)
    set_target_properties(MPQDraftDLL PROPERTIES PREFIX "")

    # Module definition file for exports
    if(MSVC)
        set_target_properties(MPQDraftDLL PROPERTIES
            LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/dll/MPQDraftDLL.def"
        )
    else()
        target_sources(MPQDraftDLL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dll/MPQDraftDLL.def)
        target_link_options(MPQDraftDLL PRIVATE "-Wl,--enable-stdcall-fixup")
    endif()

    # MPQStub - The SEMPQ stub executable
    add_executable(MPQStub WIN32
        sempq/stub/MPQStub.cpp
        sempq/stub/Stub.rc
        common/QDebug.cpp
        common/QInjectDLL.cpp
        common/QResource.cpp
        core/GameDetection.cpp
        core/GameData.cpp
    )
    configure_windows_target(MPQStub)
    add_dependencies(MPQStub MPQDraftDLL)

    install(TARGETS MPQDraftDLL MPQStub RUNTIME DESTINATION bin)
endif()

#############################################################################
# MPQDraft - The main Qt GUI application (optional, requires Qt)
#############################################################################
option(BUILD_GUI "Build the Qt GUI application" ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt - prefer Qt6, fall back to Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets Svg)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 QUIET COMPONENTS Core Widgets Svg)
    if(Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        message(STATUS "Using Qt5")
    else()
        if(BUILD_GUI)
            message(WARNING "Qt not found. GUI will not be built. Set -DBUILD_GUI=OFF to suppress this warning.")
            set(BUILD_GUI OFF)
        endif()
    endif()
endif()

if(BUILD_GUI)
    set(GUI_SOURCES
        app/gui/main_gui.cpp
        app/gui/mainwindow.cpp
        app/gui/patchwizard.cpp
        app/gui/sempqwizard.cpp
        app/gui/pluginpage.cpp
        app/gui/helpers/sempqparamsbuilder.cpp
    )

    set(CORE_SOURCES
        sempq/SEMPQCreator.cpp
        core/PluginManager.cpp
        core/GameData.cpp
        core/GameDetection.cpp
    )

    if(WIN32 OR MINGW)
        set(MAIN_SOURCES
            app/MPQDraft.cpp
            ${GUI_SOURCES}
            ${CORE_SOURCES}
            app/cli/main_cli.cpp
            app/cli/MPQDraftCLI.cpp
            app/cli/CommandParser.cpp
            common/QDebug.cpp
            common/QResource.cpp
            common/QInjectDLL.cpp
            MPQDraft.rc
        )
    else()
        set(MAIN_SOURCES ${GUI_SOURCES} ${CORE_SOURCES})
    endif()

    add_executable(MPQDraft WIN32 ${MAIN_SOURCES} app/gui/resources/mpqdraft.qrc)

    target_include_directories(MPQDraft PRIVATE
        ${COMMON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        ${CMAKE_CURRENT_SOURCE_DIR}/app/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/app/gui/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/app/cli
    )

    target_link_libraries(MPQDraft PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Svg
    )

    set_target_properties(MPQDraft PROPERTIES OUTPUT_NAME "MPQDraft-${MPQDRAFT_VERSION}")

    if(WIN32 OR MINGW)
        target_compile_definitions(MPQDraft PRIVATE MPQDRAFT_INTEGRATED_BUILD ${COMMON_DEFINITIONS})
        target_link_libraries(MPQDraft PRIVATE shlwapi psapi)
        add_dependencies(MPQDraft MPQDraftDLL MPQStub)
    endif()

    if(MINGW)
        target_compile_definitions(MPQDraft PRIVATE UNICODE _UNICODE)
        set_target_properties(MPQDraft PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    install(TARGETS MPQDraft RUNTIME DESTINATION bin)
endif()
