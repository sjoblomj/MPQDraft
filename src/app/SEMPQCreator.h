/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// SEMPQCreator.h - SEMPQ creation logic
//
// This file contains the core logic for creating Self-Executing MPQ (SEMPQ) files.

#ifndef SEMPQCREATOR_H
#define SEMPQCREATOR_H

#include <windows.h>
#include "../core/Common.h"

// Forward declarations
typedef HANDLE EFSHANDLEFORWRITE;

// Progress callback function type
// Parameters: progress (0-100), status message
typedef void (*SEMPQProgressCallback)(int progress, LPCSTR lpszStatusMessage, LPVOID lpUserData);

// Cancellation check function type
// Returns TRUE if the operation should be cancelled
typedef BOOL (*SEMPQCancellationCheck)(LPVOID lpUserData);

/////////////////////////////////////////////////////////////////////////////
// SEMPQCreator - MFC-free SEMPQ creation class
/////////////////////////////////////////////////////////////////////////////

class SEMPQCreator
{
public:
	SEMPQCreator();
	~SEMPQCreator();
	// Progress range constants (in %)
	static constexpr int WRITE_STUB_INITIAL_PROGRESS = 0;
	static constexpr int WRITE_PLUGINS_INITIAL_PROGRESS = 5;
	static constexpr int WRITE_PLUGINS_PROGRESS_SIZE = 15;
	static constexpr int WRITE_MPQ_INITIAL_PROGRESS = 20;
	static constexpr int WRITE_MPQ_PROGRESS_SIZE = 80;
	static constexpr int WRITE_FINISHED = 100;

	// Main entry point: Create a complete SEMPQ file
	// Returns TRUE on success, FALSE on failure
	BOOL CreateSEMPQ(
		// Output file path for the SEMPQ
		LPCSTR lpszOutputPath,
		// The SEMPQ name (for display in error messages)
		LPCSTR lpszSEMPQName,
		// Path to the MPQ file to embed
		LPCSTR lpszMPQPath,
		// Path to the icon file (can be NULL for default)
		LPCSTR lpszIconPath,
		// The stub data containing patch configuration
		const STUBDATA& stubData,
		// Array of plugin module structures
		const MPQDRAFTPLUGINMODULE* lpPluginModules,
		// Number of plugin modules
		DWORD nNumPluginModules,
		// Progress callback (can be NULL)
		SEMPQProgressCallback progressCallback,
		// User data passed to callbacks
		LPVOID lpUserData,
		// Cancellation check callback (can be NULL)
		SEMPQCancellationCheck cancellationCheck,
		// Output: error message buffer (must be at least 512 chars)
		LPSTR lpszErrorMessage
	);

	// Get the offset where stub data should be written in the stub executable
	// Returns 0 on failure
	static DWORD GetStubDataWriteOffset(LPCSTR lpszStubFileName);

	// Create STUBDATA structure from parameters
	// Returns dynamically allocated STUBDATA* on success, NULL on failure
	// Caller is responsible for deleting the returned pointer with: delete [] (BYTE*)pStubData
	//
	// For built-in games (registry-based):
	//   - lpszRegistryKey and lpszRegistryValue must be non-NULL
	//   - lpszTargetFileName and lpszSpawnFileName must be non-NULL
	//   - lpszTargetPath should be NULL
	//   - nShuntCount specifies DLL injection depth
	//
	// For custom executables (path-based):
	//   - lpszProgramPath must be non-NULL (full path to executable)
	//   - lpszRegistryKey and lpszRegistryValue should be NULL
	//   - lpszTargetFileName and lpszSpawnFileName are derived from lpszProgramPath
	//   - nShuntCount should be 0
	static STUBDATA* CreateStubData(
		LPCSTR lpszCustomName,           // Display name for the SEMPQ
		LPCSTR lpszRegistryKey,          // Registry key (for built-in games) or NULL
		LPCSTR lpszRegistryValue,        // Registry value (for built-in games) or NULL
		LPCSTR lpszProgramPath,          // Full path to executable (for custom) or NULL
		LPCSTR lpszTargetFileName,       // Target filename (for built-in games) or NULL
		LPCSTR lpszSpawnFileName,        // Spawn filename (for built-in games) or NULL
		LPCSTR lpszParameters,           // Command-line arguments (can be empty string)
		int nShuntCount,                 // Shunt count for DLL injection
		DWORD dwFlags,                   // Patch flags (e.g., MPQD_EXTENDED_REDIR)
		LPSTR lpszErrorMessage           // Output: error message buffer (512 chars)
	);

protected:
	// Step 1: Extract stub executable and write stub data
	BOOL WriteStubToSEMPQ(
		LPCSTR lpszEXEName,
		const STUBDATA& dataSEMPQ,
		SEMPQProgressCallback progressCallback,
		LPVOID lpUserData,
		SEMPQCancellationCheck cancellationCheck,
		LPSTR lpszErrorMessage
	);

	// Step 2: Write plugins to EFS archive
	BOOL WritePluginsToSEMPQ(
		LPCSTR lpszEXEName,
		const MPQDRAFTPLUGINMODULE* lpPluginModules,
		DWORD nNumPluginModules,
		SEMPQProgressCallback progressCallback,
		LPVOID lpUserData,
		SEMPQCancellationCheck cancellationCheck,
		LPSTR lpszErrorMessage
	);

	// Step 3: Append MPQ data to SEMPQ
	BOOL WriteMPQToSEMPQ(
		LPCSTR lpszEXEName,
		LPCSTR lpszMPQName,
		SEMPQProgressCallback progressCallback,
		LPVOID lpUserData,
		SEMPQCancellationCheck cancellationCheck,
		LPSTR lpszErrorMessage
	);

	// Helper: Write MPQ data (overload with open file handles)
	BOOL WriteMPQToSEMPQ(
		LPCSTR lpszMPQName,
		LPCSTR lpszEXEName,
		HANDLE hSEMPQ,
		HANDLE hMPQ,
		SEMPQProgressCallback progressCallback,
		LPVOID lpUserData,
		SEMPQCancellationCheck cancellationCheck,
		LPSTR lpszErrorMessage
	);

	// Helper: Report progress
	void ReportProgress(int progress, LPCSTR lpszMessage, SEMPQProgressCallback callback, LPVOID lpUserData);

	// Helper: Check for cancellation
	BOOL CheckCancellation(SEMPQCancellationCheck callback, LPVOID lpUserData);
};

#endif // SEMPQCREATOR_H
