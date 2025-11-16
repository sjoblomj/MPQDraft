/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraftCLI.h : Command-line interface for MPQDraft
//

#if !defined(AFX_MPQDRAFTCLI_H__INCLUDED_)
#define AFX_MPQDRAFTCLI_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>
#include "../Common.h"

/////////////////////////////////////////////////////////////////////////////
/*	CMPQDraftCLI
	Handles command-line interface operations for MPQDraft. This class
	encapsulates all CLI-specific functionality, including argument parsing,
	MPQ/plugin loading, and launching the patched executable. */

class CMPQDraftCLI
{
public:
	CMPQDraftCLI();
	~CMPQDraftCLI();

	// Main entry point for CLI operations
	// Returns FALSE to indicate the application should exit
	BOOL Execute(
		IN const CStringArray& params,
		IN const CStringArray& switches,
		IN LPCSTR lpszPatcherDLLPath
	);

private:
	// Parse comma-separated values into an array
	void ParseCommaSeparatedValues(
		IN LPCSTR lpszInput,
		OUT CArray<CString, CString>& output
	);

	// Load plugin modules from file paths
	BOOL LoadPluginModules(
		IN const CArray<CString, CString>& qdpPaths,
		OUT CArray<MPQDRAFTPLUGINMODULE>& modules
	);

	// Execute the patcher with the given parameters
	BOOL ExecutePatcher(
		IN LPCSTR lpszPatcherDLLPath,
		IN LPCSTR lpszProgramPath,
		IN LPCSTR lpszParameters,
		IN DWORD dwFlags,
		IN const CArray<LPCSTR>& mpqs,
		IN const CArray<MPQDRAFTPLUGINMODULE>& modules
	);
};

#endif // !defined(AFX_MPQDRAFTCLI_H__INCLUDED_)
