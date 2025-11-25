/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// PluginManager.cpp : Implementation of frontend-free plugin management

#include "pluginmanager.h"
#include "../common/patcher.h"  // For MAX_MPQDRAFT_PLUGINS, MAX_AUXILIARY_MODULES

#ifdef _WIN32
#include <windows.h>
#endif

PluginManager::~PluginManager() {
	clear();
}

// Helper function to load a plugin DLL and retrieve its information
BOOL PluginManager::LoadPluginInfo(IN LPCSTR lpszFileName, OUT PluginInfo &pluginInfo)
{
#ifdef _WIN32
    // Load the plugin's module
	pluginInfo.hDLLModule = LoadLibrary(lpszFileName);
	if (!pluginInfo.hDLLModule)
		return FALSE;

	// Get the plugin's interface
	GetMPQDraftPluginPtr pGetMPQDraftPlugin = (GetMPQDraftPluginPtr)
		GetProcAddress(pluginInfo.hDLLModule, "GetMPQDraftPlugin");

	if (!pGetMPQDraftPlugin || !pGetMPQDraftPlugin(&pluginInfo.pPlugin))
	{
		FreeLibrary(pluginInfo.hDLLModule);
		pluginInfo.hDLLModule = NULL;
		return FALSE;
	}

	// Get the plugin's ID
	pluginInfo.pPlugin->Identify(&pluginInfo.dwPluginID);

	// Get the plugin's name
	char szPluginName[255];
	pluginInfo.pPlugin->GetPluginName(szPluginName, sizeof(szPluginName));
	pluginInfo.strPluginName = szPluginName;

	// Store the filename
	pluginInfo.strFileName = lpszFileName;

	return TRUE;
#else
    // Plugin loading not supported on non-Windows platforms
    // (Plugins are Windows DLLs)
    (void)lpszFileName;
    (void)pluginInfo;
    return FALSE;
#endif
}

// Helper function to get the number of auxiliary modules for a plugin
// Returns 0 on non-Windows or if the plugin doesn't support GetModules
static int getAuxiliaryModuleCount(const PluginInfo *info)
{
#ifdef _WIN32
    if (info && info->pPlugin) {
		DWORD numModules = 0;
		if (info->pPlugin->GetModules(nullptr, &numModules)) {
			return static_cast<int>(numModules);
		}
	}
#else
    (void)info;  // Suppress unused parameter warning
#endif
    return 0;
}

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
#ifdef _WIN32
		// On Windows, this is a real error - the plugin DLL couldn't be loaded
		delete info;
		failedPlugins.insert(path);
		errorMessage = "Failed to load plugin DLL. The file may not be a valid MPQDraft plugin, "
		               "or it may be missing required dependencies.";
		return false;
#else
		// On non-Windows platforms, plugins can't be loaded (they're Windows DLLs)
		// But we can still add them to the list for testing the UI

        // TODO: Do we want to add dummy plugins?
		// Populate the PluginInfo as a dummy entry for UI testing
		info->strFileName = path;

		// Extract filename from path for display name
		size_t lastSlash = path.find_last_of("/\\");
		std::string fileName = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
		info->strPluginName = fileName;

		info->dwPluginID = 0;
		info->hDLLModule = nullptr;
		info->pPlugin = nullptr;
		// Fall through to add it to the list
#endif
	}

	// Add to our map
	loadedPlugins[path] = info;
	return true;
}

void freePluginInfo(PluginInfo *info) {
	if (info->hDLLModule) {
#ifdef _WIN32
        // On Windows, we can actually unload the DLL
        // On other platforms, we can't unload the DLL
		FreeLibrary(info->hDLLModule);
#endif
	}
	delete info;
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

bool PluginManager::configurePlugin(const std::string &path, void *hwnd) {
#ifdef _WIN32
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
#else
	// Plugin configuration is not available on non-Windows platforms
	(void)path;
	(void)hwnd;
	return false;
#endif
}

int PluginManager::getPluginModuleCount(const std::string &path) const {
	auto it = loadedPlugins.find(path);
	if (it == loadedPlugins.end()) {
		return 0;
	}

	const PluginInfo *info = it->second;
	return getAuxiliaryModuleCount(info);
}
