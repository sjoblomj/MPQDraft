#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <string>
#include <vector>

// Structure for a game component (executable)
struct GameComponent {
    std::string componentName;      // Display name (e.g., "Starcraft", "Campaign Editor")
    std::string fileName;           // File to launch (e.g., "Starcraft.exe")
    std::string targetFileName;     // Patch target filename (e.g., "Starcraft.exe")
    int shuntCount;                 // Shunt count for patching
    std::string iconPath;           // Internal resource path to icon (e.g., ":/icons/Starcraft.ico")
    bool extendedRedir;             // MPQD_EXTENDED_REDIR flag
    unsigned int flags;             // Additional flags
    std::vector<std::string> aliases;  // Aliases for this component
};

// Structure for a supported game
struct SupportedGame {
    std::string gameName;           // Display name (e.g., "Starcraft")
    std::string registryKey;        // Registry key for install path
    std::string registryValue;      // Registry value name
    std::string iconPath;           // Internal resource path to game icon
    std::vector<GameComponent> components;  // List of components
};

// Get the list of all supported games
std::vector<SupportedGame> getSupportedGames();

#endif // GAMEDATA_H
