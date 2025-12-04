#ifndef GAMEDETECTION_H
#define GAMEDETECTION_H

#include "gamedata.h"
#include <string>
#include <vector>

// Pure data about an application
struct ApplicationData {
    std::string displayText;
    std::string executablePath;
    std::string iconPath;
    std::string parameters;
    bool extendedRedir = true;
    int shuntCount = 0;
    bool noSpawning = false;
};

// Result entry with status metadata
struct ApplicationEntry {
    ApplicationData data;
    bool isOverridden = false;    // User has modified this from defaults
    bool isSupportedGame = false; // Exists in gamedata (determines "Reset" vs "Remove")
};

// Registry/game detection functions (Windows only, returns false on other platforms)

// Locates an installed game component file path through the game's registry entry
// Returns the full path to the component if found, empty string otherwise
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName);

// Get list of installed games (only games that are actually detected on the system)
std::vector<const SupportedGame*> getInstalledGames();

// Build a merged list of applications from installed games and user overrides
// - Starts with installed games from getInstalledGames()
// - Applies overrides from the provided list (matching by displayText)
// - Adds new entries for overrides that don't match any installed game
//
// The caller is responsible for providing as many sets of overrides as desired,
// and can thus use whatever source they want for overrides (Windows registry, settings files, etc.)
std::vector<ApplicationEntry> buildInstalledApplicationList(const std::vector<ApplicationData>& overrides);

#endif // GAMEDETECTION_H
