/*
    MPQDraft Qt GUI - Main Entry Point
    
    This is the Qt-based GUI for MPQDraft, replacing the old MFC GUI.
    It provides a modern, cross-platform interface while maintaining
    compatibility with the existing plugin system.
*/

#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("MPQDraft");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("MPQDraft");
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}

