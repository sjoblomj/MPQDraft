/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// PluginManager.cpp : Implementation of frontend-free plugin management

#include "pluginmanager.h"
#include "../common/patcher.h"  // For MAX_MPQDRAFT_PLUGINS, MAX_AUXILIARY_MODULES
#include <cstring>

#ifdef _WIN32
#include <windows.h>

// Helper function to load a plugin and retrieve its information
bool PluginManager::LoadPluginInfo(const char* fileName, PluginInfo &pluginInfo) {
    // Load the plugin's module
	pluginInfo.hDLLModule = LoadLibrary(fileName);
	if (!pluginInfo.hDLLModule)
		return false;

	// Get the plugin's interface
	GetMPQDraftPluginPtr pGetMPQDraftPlugin = (GetMPQDraftPluginPtr)
		GetProcAddress(pluginInfo.hDLLModule, "GetMPQDraftPlugin");

	if (!pGetMPQDraftPlugin || !pGetMPQDraftPlugin(&pluginInfo.pPlugin)) {
		FreeLibrary(pluginInfo.hDLLModule);
		pluginInfo.hDLLModule = nullptr;
		return false;
	}

	// Get the plugin's ID
	pluginInfo.pPlugin->Identify(&pluginInfo.dwPluginID);

	// Get the plugin's name
	char szPluginName[255];
	pluginInfo.pPlugin->GetPluginName(szPluginName, sizeof(szPluginName));
	pluginInfo.strPluginName = szPluginName;

	// Store the filename
	pluginInfo.strFileName = fileName;

	return true;
}

bool PluginManager::configurePlugin(const std::string &path, void *hwnd) {
    auto it = loadedPlugins.find(path);
	if (it == loadedPlugins.end()) {
		return false;
	}

	const PluginInfo *info = it->second;
	if (!info || !info->pPlugin) {
		return false;
	}

	// Cast void* back to HWND for Windows API
	HWND windowHandle = (HWND)hwnd;

	// Call the plugin's Configure method
	return info->pPlugin->Configure(windowHandle) ? true : false;
}

// Helper function to get the number of auxiliary modules for a plugin
static int getAuxiliaryModuleCount(const PluginInfo *info) {
    if (info && info->pPlugin) {
		uint32_t numModules = 0;
		if (info->pPlugin->GetModules(nullptr, &numModules)) {
			return static_cast<int>(numModules);
		}
	}
    return 0;
}

// Helper function to get auxiliary modules for a plugin
// Returns the modules in the provided vector (which should be pre-sized)
static bool getAuxiliaryModules(const PluginInfo *info, std::vector<MPQDRAFTPLUGINMODULE> &modules, int startIndex) {
    if (!info || !info->pPlugin) {
        return false;
    }

    uint32_t numModules = 0;
    if (!info->pPlugin->GetModules(nullptr, &numModules) || numModules == 0) {
        return true;  // No modules is not an error
    }

    // Get the modules directly into the vector starting at startIndex
    if (!info->pPlugin->GetModules(&modules[startIndex], &numModules)) {
        return false;
    }

    // Make all modules have the correct component ID
    for (uint32_t i = 0; i < numModules; i++) {
        modules[startIndex + i].dwComponentID = info->dwPluginID;
    }

    return true;
}

void freePluginInfo(PluginInfo *info) {
    if (info->hDLLModule) {
		FreeLibrary(info->hDLLModule);
    }
    delete info;
}

#else // Non-Windows stubs for plugin loading functions

// Helper function to load a plugin and retrieve its information
bool PluginManager::LoadPluginInfo(const char* fileName, PluginInfo &pluginInfo) {

    // Plugin loading is not supported on non-Windows platforms,
    // since plugins are Windows DLLs.
    (void)fileName;
    (void)pluginInfo;
    return false;
}

bool PluginManager::configurePlugin(const std::string &path, void *hwnd) {
    // Plugin configuration is not available on non-Windows platforms
    (void)path;
    (void)hwnd;
    return false;
}

// Helper function to get the number of auxiliary modules for a plugin
// Returns 0 on non-Windows
static int getAuxiliaryModuleCount(const PluginInfo *info) {
    (void)info;  // Suppress unused parameter warning
    return 0;
}

// Helper function to get auxiliary modules for a plugin
// Returns true on non-Windows (no modules to get)
static bool getAuxiliaryModules(const PluginInfo *info, std::vector<MPQDRAFTPLUGINMODULE> &modules, int startIndex) {
    (void)info;
    (void)modules;
    (void)startIndex;
    return true;  // No modules on non-Windows
}

// The DLL can only be unloaded on Windows.
void freePluginInfo(PluginInfo *info) {
    delete info;
}
#endif


bool PluginManager::addPlugin(const std::string &path, std::string &errorMessage) {
	// Don't add duplicates - silently filter them out
	if (loadedPlugins.find(path) != loadedPlugins.end() ||
	    failedPlugins.find(path) != failedPlugins.end()) {
		return true;  // Already handled
	}

	// Try to load the plugin
	PluginInfo *info = new PluginInfo();
	bool loadSuccess = LoadPluginInfo(path.c_str(), *info);

	if (!loadSuccess) {
		delete info;
		failedPlugins.insert(path);
		errorMessage = "Failed to load plugin '" + path + "'. The file may not be a valid MPQDraft plugin, "
		               "or it may be missing required dependencies.";
		return false;
	}

	// Add to our map
	loadedPlugins[path] = info;
	return true;
}

bool PluginManager::removePlugin(const std::string &path) {
	// Remove from loaded plugins map and clean up
	auto it = loadedPlugins.find(path);
	if (it != loadedPlugins.end()) {
		PluginInfo *info = it->second;
        freePluginInfo(info);
		loadedPlugins.erase(it);
		return true;
	}

	// Remove from failed plugins set if it's there
	auto failedIt = failedPlugins.find(path);
	if (failedIt != failedPlugins.end()) {
		failedPlugins.erase(failedIt);
		return true;
	}

	return false;
}

void PluginManager::clear() {
	// Clean up all loaded plugins
	for (auto &pair : loadedPlugins) {
		PluginInfo *info = pair.second;
        freePluginInfo(info);
	}
	loadedPlugins.clear();
	failedPlugins.clear();
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
	std::vector<std::string> result;
	result.reserve(loadedPlugins.size());
	for (const auto &pair : loadedPlugins) {
		result.push_back(pair.first);
	}
	return result;
}

std::vector<std::string> PluginManager::getFailedPlugins() const {
	std::vector<std::string> result;
	result.reserve(failedPlugins.size());
	for (const std::string &path : failedPlugins) {
		result.push_back(path);
	}
	return result;
}

const PluginInfo* PluginManager::getPluginInfo(const std::string &path) const {
	auto it = loadedPlugins.find(path);
	if (it != loadedPlugins.end()) {
		return it->second;
	}
	return nullptr;
}

bool PluginManager::isPluginLoaded(const std::string &path) const {
	return loadedPlugins.find(path) != loadedPlugins.end();
}

bool PluginManager::isPluginFailed(const std::string &path) const {
	return failedPlugins.find(path) != failedPlugins.end();
}

PluginManager::ValidationResult PluginManager::validateSelection(
	const std::vector<std::string> &selectedPaths) const {

	ValidationResult result;
	result.valid = true;
	result.selectedCount = 0;
	result.totalModules = 0;
	result.errorMessage = "";

	// Count selected plugins and modules
	for (const std::string &path : selectedPaths) {
		auto it = loadedPlugins.find(path);
		if (it == loadedPlugins.end()) {
			continue;  // Plugin not loaded, skip
		}

		result.selectedCount++;

		// Each plugin counts as 1 module (the plugin DLL itself)
		result.totalModules++;

		// Get the plugin info to count auxiliary modules
		const PluginInfo *info = it->second;
		result.totalModules += getAuxiliaryModuleCount(info);
	}

	// Check limits
	if (result.selectedCount > MAX_MPQDRAFT_PLUGINS) {
		result.valid = false;
		result.errorMessage = "Too many plugins selected. Maximum allowed: " +
		                      std::to_string(MAX_MPQDRAFT_PLUGINS);
	} else if (result.totalModules > MAX_AUXILIARY_MODULES) {
		result.valid = false;
		result.errorMessage = "Too many plugin modules. Plugins and their auxiliary modules exceed the limit. "
		                      "Maximum allowed: " + std::to_string(MAX_AUXILIARY_MODULES);
	}

	return result;
}

int PluginManager::getPluginModuleCount(const std::string &path) const {
	auto it = loadedPlugins.find(path);
	if (it == loadedPlugins.end()) {
		return 0;
	}

	const PluginInfo *info = it->second;
	return getAuxiliaryModuleCount(info);
}

std::vector<MPQDRAFTPLUGINMODULE> PluginManager::getPluginModules(const std::string &path) const {
	std::vector<MPQDRAFTPLUGINMODULE> modules;

	auto it = loadedPlugins.find(path);
	if (it == loadedPlugins.end()) {
		return modules;  // Empty vector if plugin not found
	}

	const PluginInfo *info = it->second;
	if (!info) {
		return modules;
	}

	// Get the number of auxiliary modules
	int auxModuleCount = getAuxiliaryModuleCount(info);

	// Allocate space for plugin DLL + auxiliary modules
	modules.resize(1 + auxModuleCount);

	// First entry: the plugin DLL itself (dwModuleID=0, bExecute=TRUE)
	modules[0].dwComponentID = info->dwPluginID;
	modules[0].dwModuleID = 0;
	modules[0].bExecute = 1;
	strncpy(modules[0].szModuleFileName, info->strFileName.c_str(), MPQDRAFT_MAX_PATH - 1);
	modules[0].szModuleFileName[MPQDRAFT_MAX_PATH - 1] = '\0';

	// Get auxiliary modules (if any)
	if (auxModuleCount > 0) {
		if (!getAuxiliaryModules(info, modules, 1)) {
			// Failed to get auxiliary modules - return just the plugin DLL
			modules.resize(1);
		}
	}

	return modules;
}
