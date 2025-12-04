/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Functions for locating installed games
#if !defined(GAMEDETECTION_H)
#define GAMEDETECTION_H

#include "GameData.h"
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
