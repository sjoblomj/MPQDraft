#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <QString>
#include <QVector>

// Structure for a game component (executable)
struct GameComponent {
    QString componentName;      // Display name (e.g., "Starcraft", "Campaign Editor")
    QString fileName;           // File to launch (e.g., "Starcraft.exe")
    QString targetFileName;     // Patch target filename (e.g., "Starcraft.exe")
    int shuntCount;             // Shunt count for patching
    QString iconPath;           // Qt resource path to icon (e.g., ":/icons/Starcraft.ico")
    bool extendedRedir;         // MPQD_EXTENDED_REDIR flag
    unsigned int flags;         // Additional flags
};

// Structure for a supported game
struct SupportedGame {
    QString gameName;           // Display name (e.g., "Starcraft")
    QString registryKey;        // Registry key for install path
    QString registryValue;      // Registry value name
    QString iconPath;           // Qt resource path to game icon
    QVector<GameComponent> components;  // List of components
};

// Get the list of all supported games
const QVector<SupportedGame>& getSupportedGames();

#endif // GAMEDATA_H

