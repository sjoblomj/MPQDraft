/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "GameDetection.h"
#include "PatcherFlags.h"
#include <map>
#include <set>

#ifdef _WIN32
#include <windows.h>
#include <codecvt>
#include <fstream>
#include <locale>

// Helper function to convert std::string to std::wstring on Windows
static std::wstring stringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstrTo[0], size_needed);
    return wstrTo;
}

static std::string wstringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

// Determines whether a game is installed by checking for its registry entries
// On Windows: checks both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER
// On other platforms: always returns false
static bool isGameInstalled(const std::string& registryKey, const std::string& registryValue) {
    // Convert std::string to wide string for Windows API
    std::wstring wideKey = stringToWString(registryKey);
    std::wstring wideValue = stringToWString(registryValue);

    HKEY hKey;
    DWORD dwValueType;
    wchar_t szValue[MAX_PATH + 1];
    DWORD dwValueSize = sizeof(szValue);

    // Try HKEY_CURRENT_USER first
    if (RegOpenKeyExW(HKEY_CURRENT_USER, wideKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bool found = (RegQueryValueExW(hKey, wideValue.c_str(), NULL, &dwValueType,
                                       (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        if (found) return true;
    }

    // Try HKEY_LOCAL_MACHINE
    dwValueSize = sizeof(szValue);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, wideKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bool found = (RegQueryValueExW(hKey, wideValue.c_str(), NULL, &dwValueType,
                                       (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        if (found) return true;
    }

    return false;
}

// Locates an installed game component file path through the game's registry entry
// Returns the full path to the component if found, empty string otherwise
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName) {
    // Convert std::string to wide string for Windows API
    std::wstring wideKey = stringToWString(registryKey);
    std::wstring wideValue = stringToWString(registryValue);

    HKEY hKey;
    DWORD dwValueType;
    wchar_t szValue[MAX_PATH + 1];
    DWORD dwValueSize = sizeof(szValue);

    // Try HKEY_CURRENT_USER first
    if (RegOpenKeyExW(HKEY_CURRENT_USER, wideKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, wideValue.c_str(), NULL, &dwValueType,
                            (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            std::wstring installPathW(szValue);
            std::string installPath = wstringToString(installPathW);

            // Combine path with filename
            std::string fullPath = installPath;
            if (!fullPath.empty() && fullPath.back() != '\\' && fullPath.back() != '/') {
                fullPath += '\\';
            }
            fullPath += fileName;

            if (fileExists(fullPath)) {
                return fullPath;
            }
        } else {
            RegCloseKey(hKey);
        }
    }

    // Try HKEY_LOCAL_MACHINE
    dwValueSize = sizeof(szValue);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, wideKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, wideValue.c_str(), NULL, &dwValueType,
                            (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            std::wstring installPathW(szValue);
            std::string installPath = wstringToString(installPathW);

            // Combine path with filename
            std::string fullPath = installPath;
            if (!fullPath.empty() && fullPath.back() != '\\' && fullPath.back() != '/') {
                fullPath += '\\';
            }
            fullPath += fileName;

            if (fileExists(fullPath)) {
                return fullPath;
            }
        } else {
            RegCloseKey(hKey);
        }
    }

    return std::string();
}

#else

// Determines whether a game is installed by checking for its registry entries
// On Windows: checks both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER
// On other platforms: always returns false
static bool isGameInstalled(const std::string& registryKey, const std::string& registryValue) {
    // On non-Windows platforms, registry checking is not applicable
    (void) registryKey;
    (void) registryValue;
    return false;
}

// Locates an installed game component file path through the game's registry entry.
// On non-Windows platforms, registry checking is not applicable, so this function
// just returns an empty string.
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName) {
    // On non-Windows platforms, registry checking is not applicable
    (void)registryKey;
    (void)registryValue;

    return std::string();
}
#endif

// Get list of installed games (only games that are actually detected on the system)
std::vector<const SupportedGame*> getInstalledGames() {

    // We need to store the games somewhere persistent since we're returning pointers
    static std::vector<SupportedGame> supportedGames = getSupportedGames();
    std::vector<const SupportedGame*> installedGames;

    for (const SupportedGame& game : supportedGames) {
        if (isGameInstalled(game.registryKey, game.registryValue)) {
            installedGames.push_back(&game);
        }
    }

    return installedGames;
}

// Helper to generate display name for a game component
static std::string generateDisplayName(const SupportedGame& game, const GameComponent& component) {
    if (game.components.size() == 1) {
        return game.gameName;
    }
    return game.gameName + " - " + component.componentName;
}

// Helper to check if a display name exists in gamedata (all supported games, not just installed)
static bool isDisplayNameInGameData(const std::string& displayName, std::string* outIconPath = nullptr) {
    std::vector<SupportedGame> allGames = getSupportedGames();
    for (const SupportedGame& game : allGames) {
        for (const GameComponent& component : game.components) {
            std::string name = generateDisplayName(game, component);
            if (name == displayName) {
                if (outIconPath) {
                    *outIconPath = component.iconPath;
                }
                return true;
            }
        }
    }
    return false;
}


// Build a merged list of applications from installed games and user overrides
// - Starts with installed games from getInstalledGames()
// - Applies overrides from the provided list (matching by displayText)
// - Adds new entries for overrides that don't match any installed game
//
// The caller is responsible for providing as many sets of overrides as desired,
// and can thus use whatever source they want for overrides (Windows registry, settings files, etc.)
std::vector<ApplicationEntry> buildInstalledApplicationList(const std::vector<ApplicationData>& overrides) {
    std::vector<ApplicationEntry> result;

    // Get installed games
    std::vector<const SupportedGame*> installedGames = getInstalledGames();

    // Build a map of overrides by displayText for quick lookup
    std::map<std::string, const ApplicationData*> overrideMap;
    for (const ApplicationData& override : overrides) {
        overrideMap[override.displayText] = &override;
    }

    // Track which overrides have been applied
    std::set<std::string> appliedOverrides;

    // Process installed games
    for (const SupportedGame* game : installedGames) {
        for (const GameComponent& component : game->components) {
            std::string displayName = generateDisplayName(*game, component);

            ApplicationEntry entry;
            entry.isSupportedGame = true;

            // Check if there's an override for this game
            auto it = overrideMap.find(displayName);
            if (it != overrideMap.end()) {
                // Apply override
                const ApplicationData* override = it->second;
                entry.data = *override;
                entry.isOverridden = true;
                appliedOverrides.insert(displayName);
            } else {
                // Use default values from gamedata
                std::string componentPath = locateComponent(
                        game->registryKey, game->registryValue, component.fileName);

                if (componentPath.empty()) {
                    // Skip components that can't be located
                    continue;
                }

                entry.data.displayText = displayName;
                entry.data.executablePath = componentPath;
                entry.data.iconPath = component.iconPath;
                entry.data.parameters = "";
                entry.data.extendedRedir = component.extendedRedir;
                entry.data.shuntCount = component.shuntCount;
                entry.data.noSpawning = (component.flags & MPQD_NO_SPAWNING) != 0;
                entry.isOverridden = false;
            }

            result.push_back(entry);
        }
    }

    // Add overrides that weren't applied to installed games (custom apps or uninstalled games)
    for (const ApplicationData& override : overrides) {
        if (appliedOverrides.find(override.displayText) != appliedOverrides.end()) {
            continue;  // Already processed
        }

        ApplicationEntry entry;
        entry.data = override;
        entry.isOverridden = true;

        // Check if this is a known game in gamedata (even if not installed)
        std::string defaultIconPath;
        entry.isSupportedGame = isDisplayNameInGameData(override.displayText, &defaultIconPath);

        // If no icon specified but it's in gamedata, use the default icon
        if (entry.data.iconPath.empty() && !defaultIconPath.empty()) {
            entry.data.iconPath = defaultIconPath;
        }

        result.push_back(entry);
    }

    return result;
}
