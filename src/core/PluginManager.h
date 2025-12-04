/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// PluginManager.h : Plugin management business logic
// Handles plugin loading, validation, and lifecycle management

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#ifdef _WIN32
#include <windows.h>
#else
// Stub types for non-Windows builds
typedef void* HMODULE;
#endif

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include "PatcherApi.h"

/////////////////////////////////////////////////////////////////////////////
/*	PluginInfo
	A simple structure to hold information about a loaded plugin.
	This is a lightweight, non-GUI alternative to CPluginPage::PLUGINENTRY
	that can be used by both CLI and GUI code. */

struct PluginInfo
{
    // The path of the plugin
    std::string strFileName;

    // The plugin's ID from IMPQDraftPlugin
    uint32_t dwPluginID;

    // The plugin's name from IMPQDraftPlugin
    std::string strPluginName;

    // The plugin's DLL handle (caller is responsible for freeing)
    HMODULE hDLLModule;

    // The plugin's interface (caller is responsible for cleanup)
    IMPQDraftPlugin *pPlugin;

    PluginInfo()
            : dwPluginID(0)
            , hDLLModule(nullptr)
            , pPlugin(nullptr)
    {
    }
};

/////////////////////////////////////////////////////////////////////////////
/*	PluginManager
	Manages plugin loading, validation, and lifecycle.

	This class is completely frontend-free and uses only STL containers.
	It can be used by both GUI and CLI code.

	Responsibilities:
	- Loading and unloading plugin DLLs
	- Tracking loaded and failed plugins
	- Validating plugin selection against limits
	- Providing plugin information
	- Managing plugin lifecycle (cleanup on destruction)
*/

class PluginManager {
public:
    PluginManager() = default;
    ~PluginManager() { clear(); }

    /////////////////////////////////////////////////////////////////////////////
    // Plugin Loading/Unloading

    /*	addPlugin
        Attempts to load a plugin from the specified path.

        Parameters:
            path [in] - Full path to the plugin DLL file
            errorMessage [out] - Populated with error description on failure

        Returns:
            true on success, false on failure

        Notes:
            - If the plugin is already loaded or failed, returns true (no-op)
            - On Windows, actually loads the DLL and validates it
            - On other platforms, it creates a dummy entry for UI testing
            - Failed plugins are tracked in failedPlugins set */
    bool addPlugin(const std::string &path, std::string &errorMessage);

    /*	removePlugin
        Unloads a plugin and removes it from tracking.

        Parameters:
            path [in] - Full path to the plugin to remove

        Returns:
            true if plugin was found and removed, false otherwise

        Notes:
            - Calls FreeLibrary on the DLL handle
            - Deletes the PluginInfo structure
            - Removes from both loadedPlugins and failedPlugins */
    bool removePlugin(const std::string &path);

    /*	clear
        Unloads all plugins and clears all tracking data.

        Notes:
            - Called automatically by destructor
            - Frees all DLL handles
            - Deletes all PluginInfo structures */
    void clear();

    /////////////////////////////////////////////////////////////////////////////
    // Plugin Information

    /*	getLoadedPlugins
        Returns a list of paths for all successfully loaded plugins.

        Returns:
            Vector of plugin file paths */
    std::vector<std::string> getLoadedPlugins() const;

    /*	getFailedPlugins
        Returns a list of paths for all plugins that failed to load.

        Returns:
            Vector of plugin file paths */
    std::vector<std::string> getFailedPlugins() const;

    /*	getPluginInfo
        Retrieves the PluginInfo structure for a loaded plugin.

        Parameters:
            path [in] - Full path to the plugin

        Returns:
            Pointer to PluginInfo, or nullptr if not found

        Notes:
            - The returned pointer is owned by PluginManager
            - Do not delete the returned pointer */
    const PluginInfo* getPluginInfo(const std::string &path) const;

    /*	isPluginLoaded
        Checks if a plugin is successfully loaded.

        Parameters:
            path [in] - Full path to the plugin

        Returns:
            true if plugin is loaded, false otherwise */
    bool isPluginLoaded(const std::string &path) const;

    /*	isPluginFailed
        Checks if a plugin failed to load.

        Parameters:
            path [in] - Full path to the plugin

        Returns:
            true if plugin failed to load, false otherwise */
    bool isPluginFailed(const std::string &path) const;

    /////////////////////////////////////////////////////////////////////////////
    // Validation

    /*	ValidationResult
        Structure containing the results of plugin selection validation. */
    struct ValidationResult {
        bool valid;                  // true if selection is valid
        int selectedCount;           // Number of selected plugins
        int totalModules;            // Total number of modules (plugins + auxiliary)
        std::string errorMessage;    // Error description if !valid
    };

    /*	validateSelection
        Validates a selection of plugins against MPQDraft limits.

        Parameters:
            selectedPaths [in] - Paths of plugins to validate

        Returns:
            ValidationResult structure with validation details

        Notes:
            - Checks against MAX_MPQDRAFT_PLUGINS limit
            - Checks against MAX_AUXILIARY_MODULES limit
            - On Windows, calls GetModules() to count auxiliary modules
            - On other platforms, we can only count plugins (not auxiliary modules) */
    ValidationResult validateSelection(const std::vector<std::string> &selectedPaths) const;

    /////////////////////////////////////////////////////////////////////////////
    // Plugin Operations (Windows-only)

    /*	configurePlugin
        Calls the plugin's Configure() method to show its configuration dialog.

        Parameters:
            path [in] - Full path to the plugin
            hwnd [in] - Parent window handle (HWND cast to void*)

        Returns:
            true on success, false on failure

        Notes:
            - Only works on Windows
            - Returns false on non-Windows systems
            - Plugin must be successfully loaded */
    bool configurePlugin(const std::string &path, void *hwnd);

    /*	getPluginModuleCount
        Gets the number of auxiliary modules for a plugin.

        Parameters:
            path [in] - Full path to the plugin

        Returns:
            Number of auxiliary modules, or 0 if plugin not found

        Notes:
            - Only works on Windows
            - Returns 0 on other platforms
            - Calls plugin's GetModules() method */
    int getPluginModuleCount(const std::string &path) const;

    /*	getPluginModules
        Gets all modules for a plugin (the plugin DLL itself + any auxiliary modules).

        Parameters:
            path [in] - Full path to the plugin

        Returns:
            Vector of MPQDRAFTPLUGINMODULE structures. The first entry is always
            the plugin DLL itself (with dwModuleID=0, bExecute=TRUE). Any subsequent
            entries are auxiliary data files returned by the plugin's GetModules().

        Notes:
            - Returns empty vector if plugin not found or not loaded
            - On Windows, calls plugin's GetModules() to get auxiliary modules
            - On other platforms, only returns the plugin DLL entry */
    std::vector<MPQDRAFTPLUGINMODULE> getPluginModules(const std::string &path) const;

private:
    // Map of plugin paths to their loaded info
    std::map<std::string, PluginInfo*> loadedPlugins;

    // Set of plugin paths that failed to load
    std::set<std::string> failedPlugins;

    // Helper function to load a plugin DLL and retrieve its information
    static bool LoadPluginInfo(const char* fileName, PluginInfo &pluginInfo);

    // Non-copyable
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
};

#endif // PLUGINMANAGER_H
