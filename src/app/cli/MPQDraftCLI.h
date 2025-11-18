/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.h : Command-line interface for MPQDraft
//

#if !defined(MPQDRAFTCLI_H__INCLUDED_)
#define MPQDRAFTCLI_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <vector>
#include <string>
#include "../../core/Common.h"

/////////////////////////////////////////////////////////////////////////////
/*	CMPQDraftCLI
	Handles command-line interface operations for MPQDraft. This class
	encapsulates all CLI-specific functionality, including argument parsing,
	MPQ/plugin loading, and launching the patched executable. */

void PrintVersion();
void PrintHelp();

class CMPQDraftCLI
{
public:
	// Main entry point for CLI operations
	// Returns FALSE to indicate the application should exit
	BOOL Execute(
		IN LPCSTR lpszTarget,
		IN const std::vector<std::string>& mpqs,
		IN const std::vector<std::string>& plugins,
		IN LPCSTR lpszPatcherDLLPath
	);

private:
	// Load plugin modules from file paths
	BOOL LoadPluginModules(
		IN const std::vector<std::string>& qdpPaths,
		OUT std::vector<MPQDRAFTPLUGINMODULE>& modules
	);

	// Execute the patcher with the given parameters
	BOOL ExecutePatcher(
		IN LPCSTR lpszPatcherDLLPath,
		IN LPCSTR lpszProgramPath,
		IN LPCSTR lpszParameters,
		IN DWORD dwFlags,
		IN const std::vector<const char*>& mpqs,
		IN const std::vector<MPQDRAFTPLUGINMODULE>& modules
	);
};

#endif // !defined(MPQDRAFTCLI_H__INCLUDED_)
