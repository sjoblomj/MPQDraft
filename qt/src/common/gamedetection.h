#ifndef GAMEDETECTION_H
#define GAMEDETECTION_H

#include "gamedata.h"
#include <string>
#include <vector>

// Registry/game detection functions (Windows only, returns false on other platforms)

// Determines whether a game is installed by checking for its registry entries
bool locateGame(const std::string& registryKey, const std::string& registryValue);

// Locates an installed game component file path through the game's registry entry
// Returns the full path to the component if found, empty string otherwise
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName);

// Get list of installed games (only games that are actually detected on the system)
std::vector<const SupportedGame*> getInstalledGames();

#endif // GAMEDETECTION_H
