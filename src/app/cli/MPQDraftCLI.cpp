/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.cpp : Implementation of command-line interface for MPQDraft
//

#include "../../stdafx.h"
#include "MPQDraftCLI.h"
#include "../PluginLoader.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Helper functions

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
// CMPQDraftCLI construction/destruction

CMPQDraftCLI::CMPQDraftCLI()
{
}

CMPQDraftCLI::~CMPQDraftCLI()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMPQDraftCLI implementation

BOOL CMPQDraftCLI::Execute(
	IN const std::vector<std::string>& params,
	IN const std::vector<std::string>& switches,
	IN LPCSTR lpszPatcherDLLPath
)
{
	printf("MPQDraft CLI\n");
	QDebugOut("MPQDraft CLI");

	// Validate arguments - need at least switch and 2 params (exe and mpq)
	// Plugin parameter is optional
	if (switches.size() < 1 || params.size() < 2)
	{
		printf("Usage: MPQDraft.exe -launch <exePath> <mpqFiles> [qdpFiles]\n");
		printf("  scExePath: Path to the game executable\n");
		printf("  mpqFiles: Comma-separated list of MPQ files to load\n");
		printf("  qdpFiles: (Optional) Comma-separated list of plugin files to load\n");
		QDebugOut("Usage: MPQDraft.exe -launch <scExePath> <mpqFiles> [qdpFiles]");
		return FALSE;
	}

	const std::string& action = switches[0];
	const std::string& exePathArg = params[0];
	const std::string& mpqArg = params[1];
	const std::string qdpArg = (params.size() >= 3) ? params[2] : "";

	printf("Application path: %s\n", exePathArg.c_str());
	printf("MPQ files: %s\n", mpqArg.c_str());
	printf("Plugin files: %s\n", qdpArg.c_str());
	QDebugOut("exePathArg = %s", exePathArg.c_str());
	QDebugOut("mpqArg = %s", mpqArg.c_str());
	QDebugOut("qdpArg = %s", qdpArg.c_str());

	// Parse comma-separated MPQ paths
	std::vector<std::string> mpqPaths;
	ParseCommaSeparatedValues(mpqArg.c_str(), mpqPaths);

	// Parse comma-separated QDP (plugin) paths
	std::vector<std::string> qdpPaths;
	std::vector<MPQDRAFTPLUGINMODULE> modules;

	if (!qdpArg.empty())
	{
		ParseCommaSeparatedValues(qdpArg.c_str(), qdpPaths);

		// Load plugin modules
		if (!LoadPluginModules(qdpPaths, modules))
		{
			printf("Failed to load plugin modules\n");
			QDebugOut("Failed to load plugin modules");
			return FALSE;
		}
	}
	else
	{
		QDebugOut("No plugins specified");
		printf("No plugins specified\n");
	}

	// Convert MPQ paths to LPCSTR array
	std::vector<const char*> mpqs;
	for (size_t i = 0; i < mpqPaths.size(); i++)
	{
		mpqs.push_back(mpqPaths[i].c_str());
	}

	// Compile the flags
	DWORD dwFlags = 0;
	dwFlags |= MPQD_EXTENDED_REDIR;

	// Execute the patcher
	QDebugOut("About to call ExecutePatcher with %d MPQs and %d modules", (int)mpqs.size(), (int)modules.size());
	BOOL bSuccess = ExecutePatcher(
		lpszPatcherDLLPath,
		exePathArg.c_str(),
		"",  // No additional parameters
		dwFlags,
		mpqs,
		modules
	);

	if (!bSuccess)
	{
		QDebugOut("MPQDraftPatcher failed");
		printf("MPQDraftPatcher failed\n");
	}

	return FALSE;  // Always return FALSE to exit the application
}

void CMPQDraftCLI::ParseCommaSeparatedValues(
	IN LPCSTR lpszInput,
	OUT std::vector<std::string>& output
)
{
	if (!lpszInput || lpszInput[0] == '\0')
		return;

	std::string input = lpszInput;
	size_t start = 0;
	size_t end = 0;

	while ((end = input.find(',', start)) != std::string::npos)
	{
		std::string field = input.substr(start, end - start);
		field = TrimWhitespace(field);

		if (!field.empty())
			output.push_back(field);

		start = end + 1;
	}

	// Add the last field
	if (start < input.length())
	{
		std::string field = input.substr(start);
		field = TrimWhitespace(field);

		if (!field.empty())
			output.push_back(field);
	}
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
			continue;
		}

		// Create the module entry
		MPQDRAFTPLUGINMODULE m;
		m.dwComponentID = pluginInfo.dwPluginID;
		m.dwModuleID = 0;
		m.bExecute = TRUE;
		strcpy(m.szModuleFileName, pluginInfo.strFileName.c_str());
		modules.push_back(m);

		printf("Loaded module: %s (ID: 0x%08X)\n", pluginPath.c_str(), pluginInfo.dwPluginID);
		QDebugOut("Loaded module: <%s> (ID: 0x%08X)", pluginPath.c_str(), pluginInfo.dwPluginID);

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
