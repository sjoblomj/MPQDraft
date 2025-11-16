/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.cpp : Implementation of command-line interface for MPQDraft
//

#include "../stdafx.h"
#include "MPQDraftCLI.h"
#include "../MPQDraft.h"
#include "../resource.h"
#include "../../Common/MPQDraftPlugin.h"
#include "../CommonPages.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	IN const CStringArray& params,
	IN const CStringArray& switches,
	IN LPCSTR lpszPatcherDLLPath
)
{
	printf("MPQDraft CLI\n");
	QDebugOut("MPQDraft CLI");

	// Validate arguments - need at least switch and 2 params (exe and mpq)
	// Plugin parameter is optional
	if (switches.GetCount() < 1 || params.GetCount() < 2)
	{
		printf("Usage: MPQDraft.exe -launch <scExePath> <mpqFiles> [qdpFiles]\n");
		printf("  scExePath: Path to the game executable\n");
		printf("  mpqFiles: Comma-separated list of MPQ files to load\n");
		printf("  qdpFiles: (Optional) Comma-separated list of plugin files to load\n");
		QDebugOut("Usage: MPQDraft.exe -launch <scExePath> <mpqFiles> [qdpFiles]");
		return FALSE;
	}

	CString action = switches.GetAt(0);
	CString scExePathArg = params.GetAt(0);
	CString mpqArg = params.GetAt(1);
	CString qdpArg = (params.GetCount() >= 3) ? params.GetAt(2) : "";

	printf("Action: %s\n", (LPCSTR)action);
	printf("StarCraft path: %s\n", (LPCSTR)scExePathArg);
	printf("MPQ files: %s\n", (LPCSTR)mpqArg);
	printf("Plugin files: %s\n", (LPCSTR)qdpArg);
	QDebugOut("action = %s", action);
	QDebugOut("scExePathArg = %s", scExePathArg);
	QDebugOut("mpqArg = %s", mpqArg);
	QDebugOut("qdpArg = %s", qdpArg);

	// Parse comma-separated MPQ paths
	CArray<CString, CString> mpqPaths;
	ParseCommaSeparatedValues(mpqArg, mpqPaths);

	// Parse comma-separated QDP (plugin) paths
	CArray<CString, CString> qdpPaths;
	CArray<MPQDRAFTPLUGINMODULE> modules;

	if (qdpArg.GetLength() > 0)
	{
		ParseCommaSeparatedValues(qdpArg, qdpPaths);

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
	CArray<LPCSTR> mpqs;
	for (int i = 0; i < mpqPaths.GetSize(); i++)
	{
		mpqs.Add(mpqPaths.GetAt(i));
	}

	// Compile the flags
	DWORD dwFlags = 0;
	dwFlags |= MPQD_EXTENDED_REDIR;

	// Execute the patcher
	QDebugOut("About to call ExecutePatcher with %d MPQs and %d modules", mpqs.GetSize(), modules.GetSize());
	BOOL bSuccess = ExecutePatcher(
		lpszPatcherDLLPath,
		scExePathArg,
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
	OUT CArray<CString, CString>& output
)
{
	ASSERT(lpszInput);

	int index = 0;
	CString field;
	while (AfxExtractSubString(field, lpszInput, index, _T(',')))
	{
		output.Add(field.Trim());
		++index;
	}
}

BOOL CMPQDraftCLI::LoadPluginModules(
	IN const CArray<CString, CString>& qdpPaths,
	OUT CArray<MPQDRAFTPLUGINMODULE>& modules
)
{
	for (int i = 0; i < qdpPaths.GetSize(); i++)
	{
		try
		{
			CPluginPage::PLUGINENTRY* lpPluginEntry = new CPluginPage::PLUGINENTRY(qdpPaths.GetAt(i), FALSE);
			MPQDRAFTPLUGINMODULE m;
			m.dwComponentID = lpPluginEntry->dwPluginID;
			m.dwModuleID = 0;
			m.bExecute = TRUE;
			strcpy(m.szModuleFileName, lpPluginEntry->strFileName);
			modules.Add(m);
			printf("Loaded module: %s\n", (LPCSTR)qdpPaths.GetAt(i));
			QDebugOut("Loaded module: <%s>", qdpPaths.GetAt(i));
			delete lpPluginEntry;
		}
		catch (...)
		{
			printf("ERROR: Unable to load module: %s\n", (LPCSTR)qdpPaths.GetAt(i));
			QDebugOut("ERROR: Unable to load module: <%s>", qdpPaths.GetAt(i));
		}
	}

	return TRUE;
}

BOOL CMPQDraftCLI::ExecutePatcher(
	IN LPCSTR lpszPatcherDLLPath,
	IN LPCSTR lpszProgramPath,
	IN LPCSTR lpszParameters,
	IN DWORD dwFlags,
	IN const CArray<LPCSTR>& mpqs,
	IN const CArray<MPQDRAFTPLUGINMODULE>& modules
)
{
	ASSERT(lpszPatcherDLLPath);
	ASSERT(lpszProgramPath);
	ASSERT(lpszParameters);

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
		mpqs.GetSize(),            // Number of MPQs
		modules.GetSize(),         // Number of modules
		const_cast<LPCSTR*>(mpqs.GetData()),  // MPQ array
		modules.GetData()          // Module array
	);

	return bPatchSuccess;
}
