#include "gamedetection.h"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <codecvt>
#include <locale>
#endif

// Helper function to convert std::string to std::wstring on Windows
#ifdef _WIN32
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
#endif

// Determines whether a game is installed by checking for its registry entries
// On Windows: checks both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER
// On other platforms: always returns false
bool locateGame(const std::string& registryKey, const std::string& registryValue) {
#ifdef _WIN32
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
#else
    // On non-Windows platforms, registry checking is not applicable
    (void)registryKey;
    (void)registryValue;
    return false;
#endif
}

// Locates an installed game component file path through the game's registry entry
// Returns the full path to the component if found, empty string otherwise
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName) {
#ifdef _WIN32
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
#else
    // On non-Windows platforms, registry checking is not applicable
    (void)registryKey;
    (void)registryValue;

    // TODO: Remove this hardcoded test data
    if (fileName == "Starcraft.exe") {
        return "/home/sjoblomj/bin/starcraft/StarCraft.exe";
    } else if (fileName == "Diablo II.exe") {
        return "/home/sjoblomj/bin/d2/Diablo2.exe";
    } else if (fileName == "StarEdit.exe") {
        return "/home/sjoblomj/bin/starcraft/StarEdit.exe";
    } else if (fileName == "Warcraft II BNE.exe") {
        return "/home/sjoblomj/jj/games/blizzard-files/WarCraft II/WarCraft II BNE/Warcraft II BNE.exe";
    }

    return std::string();
#endif
}

// Get list of installed games (only games that are actually detected on the system)
std::vector<const SupportedGame*> getInstalledGames() {

    // We need to store the games somewhere persistent since we're returning pointers
    static std::vector<SupportedGame> supportedGames = getSupportedGames();
    std::vector<const SupportedGame*> installedGames;

    for (const SupportedGame& game : supportedGames) {
        if (locateGame(game.registryKey, game.registryValue)) {
            installedGames.push_back(&game);
        }
    }

#ifndef _WIN32
    // On non-Windows, hardcode test games for development
    // TODO: Remove this hardcoded test data
    for (const SupportedGame& game : supportedGames) {
        if (game.gameName == "Starcraft" || game.gameName == "Diablo II" || game.gameName == "Warcraft II BNE") {
            installedGames.push_back(&game);
        }
    }
#endif

    return installedGames;
}
