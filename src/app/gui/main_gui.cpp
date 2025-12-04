/*
    MPQDraft Qt GUI - GUI Entry Point

    This is the Qt-based GUI for MPQDraft, replacing the old MFC GUI.
    It provides a modern, cross-platform interface while maintaining
    compatibility with the existing plugin system.
*/

#include "main_gui.h"
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include "mainwindow.h"

// Core Qt GUI initialization - used by both standalone and integrated builds
static int runQtGuiCore(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    // Use same organization name as original MFC app for registry compatibility on Windows
    QApplication::setOrganizationName("Team MoPaQ");
    QApplication::setApplicationName("MPQDraft");
    QApplication::setApplicationVersion("1.0");

    // Set the application icon (window and taskbar)
    QIcon appIcon(":/icons/mpqdraft.ico");
    app.setWindowIcon(appIcon);

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

// Entry point for integrated build (called from MPQDraft.cpp)
int runQtGui(int argc, char *argv[])
{
    return runQtGuiCore(argc, argv);
}

// Standalone entry point for Qt-only builds (CMake/Linux development)
#ifndef MPQDRAFT_INTEGRATED_BUILD
int main(int argc, char *argv[])
{
    return runQtGuiCore(argc, argv);
}
#endif
