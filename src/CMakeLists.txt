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
find_package(Qt6 QUIET COMPONENTS Core Widgets Svg LinguistTools)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 QUIET COMPONENTS Core Widgets Svg LinguistTools)
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

    # Translation files (automatically detect all .ts files)
    file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/app/gui/translations/*.ts")

    # Create .qm files from .ts files
    if(Qt6_FOUND)
        qt6_add_translation(QM_FILES ${TS_FILES})
    else()
        qt5_add_translation(QM_FILES ${TS_FILES})
    endif()

    # Generate a qrc file at build time that embeds all .qm files
    # This is required because qt5_add_translation / qt6_add_translation
    # do not embed the .qm files into the executable's resources on their own.
    set(TRANSLATIONS_QRC "${CMAKE_CURRENT_BINARY_DIR}/translations_embedded.qrc")
    file(WRITE ${TRANSLATIONS_QRC}
        "<!DOCTYPE RCC>\n"
        "<RCC version=\"1.0\">\n"
        "    <qresource prefix=\"/translations\">\n"
    )
    foreach(QM_FILE ${QM_FILES})
        get_filename_component(QM_NAME ${QM_FILE} NAME)
        file(APPEND ${TRANSLATIONS_QRC} "        <file>${QM_NAME}</file>\n")
    endforeach()
    file(APPEND ${TRANSLATIONS_QRC} "    </qresource>\n</RCC>\n")

    if(Qt6_FOUND)
        qt6_add_resources(MPQDraft_TRANSLATIONS_QRC ${TRANSLATIONS_QRC})
    else()
        qt5_add_resources(MPQDraft_TRANSLATIONS_QRC ${TRANSLATIONS_QRC})
    endif()

    # Add translations to the target
    target_sources(MPQDraft PRIVATE ${MPQDraft_TRANSLATIONS_QRC})

    # Install translation files
    install(FILES ${QM_FILES} DESTINATION translations)

    # Copy Qt's built-in translations for GUI controls (QWizard buttons, etc.)
    if(Qt6_FOUND)
        set(QT_TRANSLATIONS_DIR ${Qt6Core_DIR}/../../../translations)
    else()
        set(QT_TRANSLATIONS_DIR ${Qt5Core_DIR}/../../../translations)
    endif()
    set(QT_QM_FILES
        ${QT_TRANSLATIONS_DIR}/qt_zh_CN.qm
        ${QT_TRANSLATIONS_DIR}/qt_ko_KR.qm
        ${QT_TRANSLATIONS_DIR}/qt_sv_SE.qm
    )
    foreach(QT_QM ${QT_QM_FILES})
        if(EXISTS ${QT_QM})
            get_filename_component(QT_QM_NAME ${QT_QM} NAME)
            configure_file(${QT_QM} ${CMAKE_CURRENT_BINARY_DIR}/translations/${QT_QM_NAME} COPYONLY)
            install(FILES ${QT_QM} DESTINATION translations)
        endif()
    endforeach()

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
