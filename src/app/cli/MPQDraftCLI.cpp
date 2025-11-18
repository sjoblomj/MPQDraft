/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.cpp : Implementation of command-line interface for MPQDraft
//

#include "stdafx_cli.h"
#include "MPQDraftCLI.h"
#include "CommandParser.h"
#include "../PluginLoader.h"
#include "../resource_ids.h"
#include "version.h"

/////////////////////////////////////////////////////////////////////////////
// Helper functions

void PrintVersion()
{
	printf("MPQDraft %s\n", MPQDRAFT_VERSION);
	printf("  By Quantam (Justin Olbrantz)\n");
	printf("  Updated by milestone-dev and Ojan (Johan Sjöblom)\n");
}

void PrintHelp()
{
	printf("MPQDraft CLI - Command-line interface for MPQDraft\n\n");
	printf("Usage: MPQDraft.exe --target <exePath> [--mpq <mpqFile>]... [--plugin <pluginFile>]...\n\n");
	printf("Options:\n");
	printf("  -t, --target <exe>    Target executable to patch and launch\n");
	printf("  -m, --mpq <mpq>       MPQ archive to load (can be specified multiple times)\n");
	printf("  -p, --plugin <qdp>    Plugin to load (can be specified multiple times)\n");
	printf("  -h, --help            Show this help message\n");
	printf("  -v, --version         Show version information\n");
	printf("\nAt least one MPQ or plugin must be specified.\n");
	printf("\nExamples:\n");
	printf("  MPQDraft.exe --target \"C:\\Starcraft\\StarCraft.exe\" --mpq \"C:\\Mod\\my_mod_1.mpq\" --mpq \"C:\\Mod\\my_mod_2.mpq\" --plugin \"C:\\Mod\\my_plugin_1.qdp\" --plugin \"C:\\Mod\\my_plugin_2.qdp\"\n");
}

// Trim whitespace from both ends of a string
static std::string TrimWhitespace(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";

	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, last - first + 1);
}

/////////////////////////////////////////////////////////////////////////////
// CMPQDraftCLI implementation

BOOL CMPQDraftCLI::Execute(
	IN LPCSTR lpszTarget,
	IN const std::vector<std::string>& mpqs,
	IN const std::vector<std::string>& plugins,
	IN LPCSTR lpszPatcherDLLPath
)
{
	printf("MPQDraft CLI\n");
	QDebugOut("MPQDraft CLI");

	// Validate arguments - need target and at least one MPQ or plugin
	if (!lpszTarget || lpszTarget[0] == '\0' || (mpqs.empty() && plugins.empty()))
	{
		if (mpqs.empty() && plugins.empty())
		{
			QDebugOut("Error: At least one MPQ or plugin must be specified");
			printf("Error: At least one MPQ or plugin must be specified\n\n");
		}
		else {
			QDebugOut("Error: No target executable specified");
			printf("Error: No target executable specified\n\n");
		}
		PrintHelp();
		return FALSE;
	}

	printf("Target executable: %s\n", lpszTarget);
	printf("MPQ files (%d):\n", (int)mpqs.size());
	for (size_t i = 0; i < mpqs.size(); i++)
	{
		printf("  [%d] %s\n", (int)i, mpqs[i].c_str());
	}
	printf("Plugin files (%d):\n", (int)plugins.size());
	for (size_t i = 0; i < plugins.size(); i++)
	{
		printf("  [%d] %s\n", (int)i, plugins[i].c_str());
	}

	QDebugOut("Target = %s", lpszTarget);
	QDebugOut("MPQs = %d", (int)mpqs.size());
	QDebugOut("Plugins = %d", (int)plugins.size());

	// Load plugin modules
	std::vector<MPQDRAFTPLUGINMODULE> modules;
	if (!plugins.empty())
	{
		if (!LoadPluginModules(plugins, modules))
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
	for (size_t i = 0; i < mpqs.size(); i++)
	{
		mpqPtrs.push_back(mpqs[i].c_str());
	}

	// Compile the flags
	DWORD dwFlags = 0;
	dwFlags |= MPQD_EXTENDED_REDIR;

	// Execute the patcher
	QDebugOut("About to call ExecutePatcher with %d MPQs and %d modules", (int)mpqPtrs.size(), (int)modules.size());
	BOOL bSuccess = ExecutePatcher(
		lpszPatcherDLLPath,
		lpszTarget,
		"",  // No additional parameters
		dwFlags,
		mpqPtrs,
		modules
	);

	if (!bSuccess)
	{
		QDebugOut("MPQDraftPatcher failed");
		printf("MPQDraftPatcher failed\n");
	}

	return FALSE;  // Always return FALSE to exit the application
}


BOOL CMPQDraftCLI::LoadPluginModules(
	IN const std::vector<std::string>& qdpPaths,
	OUT std::vector<MPQDRAFTPLUGINMODULE>& modules
)
{
	for (size_t i = 0; i < qdpPaths.size(); i++)
	{
		const std::string& pluginPath = qdpPaths[i];

		// Load the plugin using the shared utility
		PluginInfo pluginInfo;
		if (!LoadPluginInfo(pluginPath.c_str(), pluginInfo))
		{
			printf("ERROR: Unable to load plugin: %s\n", pluginPath.c_str());
			QDebugOut("ERROR: Unable to load plugin: <%s>", pluginPath.c_str());
			return FALSE;
		}

		printf("Loaded plugin: %s (ID: 0x%08X, Name: %s)\n",
			pluginPath.c_str(),
			pluginInfo.dwPluginID,
			pluginInfo.strPluginName.c_str());
		QDebugOut("Loaded plugin: <%s> (ID: 0x%08X, Name: %s)",
			pluginPath.c_str(),
			pluginInfo.dwPluginID,
			pluginInfo.strPluginName.c_str());

		// Check if the plugin is ready for patching
		if (!pluginInfo.pPlugin->ReadyForPatch())
		{
			printf("ERROR: Plugin '%s' is not configured or not ready for patching.\n",
				pluginInfo.strPluginName.c_str());
			printf("       Please run the GUI version of MPQDraft to configure this plugin first.\n");
			QDebugOut("ERROR: Plugin '%s' is not ready for patching", pluginInfo.strPluginName.c_str());
			return FALSE;
		}

		printf("Plugin '%s' is ready for patching.\n", pluginInfo.strPluginName.c_str());
		QDebugOut("Plugin '%s' is ready for patching", pluginInfo.strPluginName.c_str());

		// Create the module entry
		MPQDRAFTPLUGINMODULE m;
		m.dwComponentID = pluginInfo.dwPluginID;
		m.dwModuleID = 0;
		m.bExecute = TRUE;
		strcpy(m.szModuleFileName, pluginInfo.strFileName.c_str());
		modules.push_back(m);

		// Note: We don't call FreeLibrary here because the DLL needs to stay loaded
		// for the patcher to use it
	}

	return TRUE;
}

BOOL CMPQDraftCLI::ExecutePatcher(
	IN LPCSTR lpszPatcherDLLPath,
	IN LPCSTR lpszProgramPath,
	IN LPCSTR lpszParameters,
	IN DWORD dwFlags,
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
		0,                         // Shunt count
		(DWORD)mpqs.size(),        // Number of MPQs
		(DWORD)modules.size(),     // Number of modules
		mpqs.empty() ? NULL : const_cast<LPCSTR*>(&mpqs[0]),  // MPQ array
		modules.empty() ? NULL : const_cast<MPQDRAFTPLUGINMODULE*>(&modules[0])  // Module array
	);

	return bPatchSuccess;
}
