/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "GameDetection.h"

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
