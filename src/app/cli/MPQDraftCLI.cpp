/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.cpp : Implementation of command-line interface for MPQDraft
//

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../../common/QDebug.h"
#include "MPQDraftCLI.h"
#include "CommandParser.h"
#include "../../core/PluginManager.h"
#include "../../core/GameData.h"
#include "../../core/PatcherFlags.h"
#include "../../sempq/SEMPQCreator.h"
#include "../resource_ids.h"
#include "version.h"

/////////////////////////////////////////////////////////////////////////////
// Helper functions

// Helper for case-insensitive string comparison
static std::string toLower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

// Static storage for persistent game data (needed for returning pointers)
static std::vector<SupportedGame> s_persistentGames;
static bool s_gamesInitialized = false;

// Find a game component by alias (case-insensitive)
static bool findGameByAlias(const std::string& alias,
                            const SupportedGame** outGame,
                            const GameComponent** outComponent) {
	if (!s_gamesInitialized) {
		s_persistentGames = getSupportedGames();
		s_gamesInitialized = true;
	}

	std::string lowerAlias = toLower(alias);

	for (const auto& game : s_persistentGames) {
		for (const auto& comp : game.components) {
			for (const auto& compAlias : comp.aliases) {
				if (toLower(compAlias) == lowerAlias) {
					*outGame = &game;
					*outComponent = &comp;
					return true;
				}
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
// CMPQDraftCLI implementation

BOOL CMPQDraftCLI::ExecutePatch(
	IN const PatchCommand& cmd,
	IN LPCSTR lpszPatcherDLLPath
)
{
	printf("MPQDraft CLI - Patch Mode\n");
	QDebugOut("MPQDraft CLI - Patch Mode");

	// Print configuration
	printf("Target executable: %s\n", cmd.target.c_str());
	if (!cmd.parameters.empty())
		printf("Parameters: %s\n", cmd.parameters.c_str());
	printf("Extended redirection: %s\n", cmd.extendedRedir ? "enabled" : "disabled");
	printf("No spawning: %s\n", cmd.noSpawning ? "enabled" : "disabled");
	printf("Shunt count: %d\n", cmd.shuntCount);

	printf("MPQ files (%d):\n", (int)cmd.mpqs.size());
	for (size_t i = 0; i < cmd.mpqs.size(); i++)
	{
		printf("  [%d] %s\n", (int)i, cmd.mpqs[i].c_str());
	}
	printf("Plugin files (%d):\n", (int)cmd.plugins.size());
	for (size_t i = 0; i < cmd.plugins.size(); i++)
	{
		printf("  [%d] %s\n", (int)i, cmd.plugins[i].c_str());
	}

	QDebugOut("Target = %s", cmd.target.c_str());
	QDebugOut("MPQs = %d", (int)cmd.mpqs.size());
	QDebugOut("Plugins = %d", (int)cmd.plugins.size());

	// Load plugin modules
	std::vector<MPQDRAFTPLUGINMODULE> modules;
	if (!cmd.plugins.empty())
	{
		if (!LoadPluginModules(cmd.plugins, modules))
		{
			printf("Failed to load plugin modules\n");
			QDebugOut("Failed to load plugin modules");
			return FALSE;
		}
	}
	else
	{
		QDebugOut("No plugins specified");
	}

	// Convert MPQ paths to LPCSTR array
	std::vector<const char*> mpqPtrs;
	for (size_t i = 0; i < cmd.mpqs.size(); i++)
	{
		mpqPtrs.push_back(cmd.mpqs[i].c_str());
	}

	// Compile the flags
	DWORD dwFlags = 0;
	if (cmd.extendedRedir)
		dwFlags |= MPQD_EXTENDED_REDIR;
	if (cmd.noSpawning)
		dwFlags |= MPQD_NO_SPAWNING;

	// Execute the patcher
	QDebugOut("About to call ExecutePatcher with %d MPQs and %d modules", (int)mpqPtrs.size(), (int)modules.size());
	BOOL bSuccess = ExecutePatcher(
		lpszPatcherDLLPath,
		cmd.target.c_str(),
		cmd.parameters.c_str(),
		dwFlags,
		cmd.shuntCount,
		mpqPtrs,
		modules
	);

	if (!bSuccess)
	{
		QDebugOut("MPQDraftPatcher failed");
		printf("MPQDraftPatcher failed\n");
	}

	return bSuccess;
}


BOOL CMPQDraftCLI::LoadPluginModules(
	IN const std::vector<std::string>& qdpPaths,
	OUT std::vector<MPQDRAFTPLUGINMODULE>& modules
)
{
	PluginManager pluginManager;

	for (size_t i = 0; i < qdpPaths.size(); i++)
	{
		const std::string& pluginPath = qdpPaths[i];

		// Load the plugin
		std::string errorMessage;
		if (!pluginManager.addPlugin(pluginPath, errorMessage))
		{
			printf("ERROR: Unable to load plugin: %s\n", pluginPath.c_str());
			printf("       %s\n", errorMessage.c_str());
			QDebugOut("ERROR: Unable to load plugin: <%s>", pluginPath.c_str());
			return FALSE;
		}

		const PluginInfo* pluginInfo = pluginManager.getPluginInfo(pluginPath);
		if (!pluginInfo)
		{
			printf("ERROR: Failed to get plugin info for: %s\n", pluginPath.c_str());
			QDebugOut("ERROR: Failed to get plugin info for: <%s>", pluginPath.c_str());
			return FALSE;
		}

		printf("Loaded plugin: %s (ID: 0x%08X, Name: %s)\n",
			pluginPath.c_str(),
			pluginInfo->dwPluginID,
			pluginInfo->strPluginName.c_str());
		QDebugOut("Loaded plugin: <%s> (ID: 0x%08X, Name: %s)",
			pluginPath.c_str(),
			pluginInfo->dwPluginID,
			pluginInfo->strPluginName.c_str());

		// Check if the plugin is ready for patching
		if (!pluginInfo->pPlugin->ReadyForPatch())
		{
			printf("ERROR: Plugin '%s' is not configured or not ready for patching.\n",
				pluginInfo->strPluginName.c_str());
			printf("       Please run the GUI version of MPQDraft to configure this plugin first.\n");
			QDebugOut("ERROR: Plugin '%s' is not ready for patching", pluginInfo->strPluginName.c_str());
			return FALSE;
		}

		printf("Plugin '%s' is ready for patching.\n", pluginInfo->strPluginName.c_str());
		QDebugOut("Plugin '%s' is ready for patching", pluginInfo->strPluginName.c_str());

		// Get all modules for this plugin (plugin DLL + any auxiliary modules)
		std::vector<MPQDRAFTPLUGINMODULE> pluginModules = pluginManager.getPluginModules(pluginPath);
		for (const auto& m : pluginModules)
		{
			modules.push_back(m);
		}
	}

	// Note: PluginManager's destructor will call FreeLibrary on the plugin DLLs.
	// This is fine because the patcher spawns a new process and loads the DLLs
	// there based on the file paths in the MPQDRAFTPLUGINMODULE entries.

	return TRUE;
}

BOOL CMPQDraftCLI::ExecutePatcher(
	IN LPCSTR lpszPatcherDLLPath,
	IN LPCSTR lpszProgramPath,
	IN LPCSTR lpszParameters,
	IN DWORD dwFlags,
	IN int shuntCount,
	IN const std::vector<const char*>& mpqs,
	IN const std::vector<MPQDRAFTPLUGINMODULE>& modules
)
{
	if (!lpszPatcherDLLPath || !lpszProgramPath || !lpszParameters)
		return FALSE;

	// Load the patcher DLL
	HMODULE hDLL = GetModuleHandle(lpszPatcherDLLPath);
	if (!hDLL)
	{
		hDLL = LoadLibrary(lpszPatcherDLLPath);
		if (!hDLL)
		{
			printf("Failed to load patcher DLL: %s\n", lpszPatcherDLLPath);
			QDebugOut("Failed to load patcher DLL: %s", lpszPatcherDLLPath);
			return FALSE;
		}
	}

	// Get the patcher function
	MPQDraftPatcherPtr MPQDraftPatcher = (MPQDraftPatcherPtr)GetProcAddress(hDLL, "MPQDraftPatcher");
	if (!MPQDraftPatcher)
	{
		printf("Failed to get MPQDraftPatcher function\n");
		QDebugOut("Failed to get MPQDraftPatcher function");
		return FALSE;
	}

	// Build the command line from the program and parameters
	char szCommandLine[MAX_PATH * 2];
	char szCurDir[MAX_PATH + 1];
	char szStartDir[MAX_PATH + 1];

	// Get the startup directory (directory of the target program)
	strcpy(szStartDir, lpszProgramPath);
	PathRemoveFileSpec(szStartDir);

	// Get the current directory (directory of MPQDraft)
	GetModuleFileName(NULL, szCurDir, MAX_PATH);
	PathRemoveFileSpec(szCurDir);

	// Build the command line
	wsprintf(szCommandLine, "\"%s\" %s", lpszProgramPath, lpszParameters);

	// Use the same environment as for MPQDraft
	STARTUPINFO si;
	GetStartupInfo(&si);

	// Execute the patcher
	BOOL bPatchSuccess = MPQDraftPatcher(
		lpszProgramPath,           // Application name
		szCommandLine,             // Command line
		NULL,                      // Process attributes
		NULL,                      // Thread attributes
		FALSE,                     // Inherit handles
		0,                         // Creation flags
		NULL,                      // Environment
		szStartDir,                // Current directory
		&si,                       // Startup info
		dwFlags,                   // Patching flags
		szCurDir,                  // MPQDraft directory
		lpszProgramPath,           // Spawn path (same as target)
		(DWORD)shuntCount,         // Shunt count
		(DWORD)mpqs.size(),        // Number of MPQs
		(DWORD)modules.size(),     // Number of modules
		mpqs.empty() ? NULL : const_cast<LPCSTR*>(&mpqs[0]),  // MPQ array
		modules.empty() ? NULL : const_cast<MPQDRAFTPLUGINMODULE*>(&modules[0])  // Module array
	);

	return bPatchSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// ExecuteSEMPQ - Create a Self-Executing MPQ

BOOL CMPQDraftCLI::ExecuteSEMPQ(IN const SEMPQCommand& cmd)
{
	printf("MPQDraft CLI - SEMPQ Creation Mode\n");
	QDebugOut("MPQDraft CLI - SEMPQ Creation Mode");

	// Print configuration
	printf("Output: %s\n", cmd.outputPath.c_str());
	printf("Name: %s\n", cmd.sempqName.c_str());
	printf("MPQ: %s\n", cmd.mpqPath.c_str());

	switch (cmd.mode)
	{
		case SEMPQTargetMode::SupportedGame:
			printf("Mode: Supported Game\n");
			printf("  Game alias: %s\n", cmd.gameName.c_str());
			break;

		case SEMPQTargetMode::CustomRegistry:
			printf("Mode: Custom Registry\n");
			printf("  Registry Key: %s\n", cmd.registryKey.c_str());
			printf("  Registry Value: %s\n", cmd.registryValue.c_str());
			printf("  Full Path: %s\n", cmd.fullPath ? "yes" : "no");
			if (!cmd.fullPath)
			{
				printf("  Exe File: %s\n", cmd.exeFileName.c_str());
				printf("  Target File: %s\n", cmd.targetFileName.c_str());
			}
			break;

		case SEMPQTargetMode::CustomTarget:
			printf("Mode: Custom Target\n");
			printf("  Target: %s\n", cmd.targetPath.c_str());
			break;
	}

	if (!cmd.parameters.empty())
		printf("Parameters: %s\n", cmd.parameters.c_str());
	printf("Extended redirection: %s\n", cmd.extendedRedir ? "enabled" : "disabled");
	printf("No spawning: %s\n", cmd.noSpawning ? "enabled" : "disabled");
	printf("Shunt count: %d\n", cmd.shuntCount);
	if (!cmd.iconPath.empty())
		printf("Icon: %s\n", cmd.iconPath.c_str());

	printf("Plugin files (%d):\n", (int)cmd.plugins.size());
	for (size_t i = 0; i < cmd.plugins.size(); i++)
	{
		printf("  [%d] %s\n", (int)i, cmd.plugins[i].c_str());
	}

	// Build SEMPQCreationParams
	SEMPQCreationParams params;
	params.outputPath = cmd.outputPath;
	params.sempqName  = cmd.sempqName;
	params.mpqPath    = cmd.mpqPath;
	params.iconPath   = cmd.iconPath;
	params.parameters = cmd.parameters;
	params.shuntCount = cmd.shuntCount;

	// Build flags
	params.flags = 0;
	if (cmd.extendedRedir)
		params.flags |= MPQD_EXTENDED_REDIR;
	if (cmd.noSpawning)
		params.flags |= MPQD_NO_SPAWNING;

	// Set up target based on mode
	switch (cmd.mode)
	{
		case SEMPQTargetMode::SupportedGame:
		{
			// Find the game by alias
			const SupportedGame* game = nullptr;
			const GameComponent* comp = nullptr;

			if (!findGameByAlias(cmd.gameName, &game, &comp))
			{
				printf("ERROR: Game alias '%s' not found\n", cmd.gameName.c_str());
				return FALSE;
			}

			params.useRegistry = true;
			params.registryKey = game->registryKey;
			params.registryValue = game->registryValue;
			params.valueIsFullPath = false;
			params.spawnFileName = comp->fileName;
			params.targetFileName = comp->targetFileName;
			params.shuntCount = comp->shuntCount;

			// Apply component's default flags if not overridden
			if (comp->extendedRedir && !cmd.extendedRedir)
			{
				// User explicitly disabled, keep disabled
			}
			else if (comp->extendedRedir)
			{
				params.flags |= MPQD_EXTENDED_REDIR;
			}
			break;
		}

		case SEMPQTargetMode::CustomRegistry:
			params.useRegistry = true;
			params.registryKey = cmd.registryKey;
			params.registryValue = cmd.registryValue;
			params.valueIsFullPath = cmd.fullPath;
			params.spawnFileName = cmd.exeFileName;
			params.targetFileName = cmd.targetFileName;
			break;

		case SEMPQTargetMode::CustomTarget:
			params.useRegistry = false;
			params.targetPath = cmd.targetPath;
			break;
	}

	// Load plugins
	if (!cmd.plugins.empty())
	{
		if (!LoadPluginModules(cmd.plugins, params.pluginModules))
		{
			printf("Failed to load plugin modules\n");
			QDebugOut("Failed to load plugin modules");
			return FALSE;
		}
	}

	// Progress callback
	auto progressCallback = [](int progress, const std::string& status) {
		printf("[%3d%%] %s", progress, status.c_str());
	};

	// Cancellation check (always return false - no cancellation in CLI)
	auto cancellationCheck = []() { return false; };

	// Create the SEMPQ
	SEMPQCreator creator;
	std::string errorMessage;

	printf("\nCreating SEMPQ...\n");
	bool success = creator.createSEMPQ(params, progressCallback, cancellationCheck, errorMessage);

	if (!success)
	{
		printf("\nERROR: Failed to create SEMPQ: %s\n", errorMessage.c_str());
		QDebugOut("Failed to create SEMPQ: %s", errorMessage.c_str());
		return FALSE;
	}

	printf("\nSEMPQ created successfully: %s\n", cmd.outputPath.c_str());
	return TRUE;
}
