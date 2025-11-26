/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// SEMPQCreator.cpp - SEMPQ creation implementation

#include "SEMPQCreator.h"
#include "../common/QResource.h"
#include "../app/resource_ids.h"
#include <stdio.h>
#include <shlwapi.h>

/////////////////////////////////////////////////////////////////////////////
// SEMPQCreator implementation
/////////////////////////////////////////////////////////////////////////////

BOOL SEMPQCreator::CreateSEMPQ(
	LPCSTR lpszOutputPath,
	LPCSTR lpszSEMPQName,
	LPCSTR lpszMPQPath,
	LPCSTR lpszIconPath,
	const STUBDATA& stubData,
	const MPQDRAFTPLUGINMODULE* lpPluginModules,
	DWORD nNumPluginModules,
	SEMPQProgressCallback progressCallback,
	LPVOID lpUserData,
	SEMPQCancellationCheck cancellationCheck,
	LPSTR lpszErrorMessage)
{
	// Validate parameters
	if (!lpszOutputPath || !lpszMPQPath || !lpszErrorMessage)
	{
		if (lpszErrorMessage)
			strcpy(lpszErrorMessage, "Invalid parameters");
		return FALSE;
	}

	// Check if MPQ file exists
	DWORD dwAttrib = GetFileAttributes(lpszMPQPath);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		sprintf(lpszErrorMessage, "The MPQ file does not exist: %s", lpszMPQPath);
		return FALSE;
	}

	// Step 1: Write stub to SEMPQ
	if (!WriteStubToSEMPQ(lpszOutputPath, stubData, progressCallback, lpUserData, cancellationCheck, lpszErrorMessage))
		return FALSE;

	// Step 2: Write plugins to SEMPQ
	if (!WritePluginsToSEMPQ(lpszOutputPath, lpPluginModules, nNumPluginModules, progressCallback, lpUserData, cancellationCheck, lpszErrorMessage))
		return FALSE;

	// Step 3: Write MPQ to SEMPQ
	if (!WriteMPQToSEMPQ(lpszOutputPath, lpszMPQPath, progressCallback, lpUserData, cancellationCheck, lpszErrorMessage))
		return FALSE;

	// Success!
	ReportProgress(WRITE_FINISHED, "SEMPQ created successfully!", progressCallback, lpUserData);
	return TRUE;
}

DWORD SEMPQCreator::GetStubDataWriteOffset(LPCSTR lpszStubFileName)
{
	// Manually finding the address of the stub data in an executable is to
	// be avoided, where possible. Fortunately, there's a clever way to get
	// this information through Windows: the LOAD_LIBRARY_AS_DATAFILE flag.
	// When this flag is used to load a module, the file is mapped into
	// memory as a single chunk, in exactly the same format as the file
	// itself. Some Windows functions (including the resource functions) are
	// able to operate on a file loaded in this way. In this case, the RVA of
	// the resource corresponds exactly to the file offset.
	if (!lpszStubFileName || !*lpszStubFileName)
		return 0;

	// Load the stub as a data file
	HMODULE hStub = LoadLibraryEx(lpszStubFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!hStub)
		return 0;

	DWORD dwRetVal = 0;

	// The three steps to getting a resource pointer: find, load, and lock
	HRSRC hStubData = FindResource(hStub, "STUBDATA", "BIN");
	if (hStubData)
	{
		HGLOBAL hStubDataGlobal = LoadResource(hStub, hStubData);
		if (hStubDataGlobal)
		{
			LPVOID lpvStubData = LockResource(hStubDataGlobal);
			DWORD dwStubDataSize = SizeofResource(hStub, hStubData);

			if (lpvStubData && dwStubDataSize >= STUBDATASIZE)
				// Got it
				// Note that in Windows modules always begin at 64 KB offsets,
				// which leaves these 16 bits in the HMODULE free for Windows
				// to use for flags (though I only know of one bit that is
				// used in that way); consequently, we must filter out these
				// flags before we subtract.
				dwRetVal = (DWORD)((UINT_PTR)lpvStubData
					- ((UINT_PTR)hStub & ~(UINT_PTR)0xFFFF));
		}
	}

	FreeLibrary(hStub);

	return dwRetVal;
}

BOOL SEMPQCreator::WriteStubToSEMPQ(
	LPCSTR lpszEXEName,
	const STUBDATA& dataSEMPQ,
	SEMPQProgressCallback progressCallback,
	LPVOID lpUserData,
	SEMPQCancellationCheck cancellationCheck,
	LPSTR lpszErrorMessage)
{
	ReportProgress(WRITE_STUB_INITIAL_PROGRESS, "Writing Executable Code...", progressCallback, lpUserData);

	// We've got a couple tasks to do here. First, we need to create the SEMPQ
	// file and write the unmodified version of the stub.
	if (!ExtractResource(NULL, MAKEINTRESOURCE(IDR_SEMPQSTUB), "EXE", lpszEXEName))
	{
		sprintf(lpszErrorMessage, "Unable to create file: %s", lpszEXEName);
		return FALSE;
	}

	// Next, create the new stub data that contains the info for our mod
	DWORD dwStubDataOffset = GetStubDataWriteOffset(lpszEXEName);
	if (!dwStubDataOffset)
	{
		strcpy(lpszErrorMessage, "Internal error: unable to locate stub data offset");
		return FALSE;
	}

	// Open the file, and...
	HANDLE hSEMPQ = CreateFile(lpszEXEName,
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSEMPQ == INVALID_HANDLE_VALUE)
	{
		sprintf(lpszErrorMessage, "Unable to open file: %s", lpszEXEName);
		return FALSE;
	}

	// Write the stub data
	BOOL bRetVal = FALSE;
	DWORD dwBytesWritten;
	if (SetFilePointer(hSEMPQ, dwStubDataOffset, NULL, FILE_BEGIN) == dwStubDataOffset
		&& WriteFile(hSEMPQ, &dataSEMPQ, dataSEMPQ.cbSize, &dwBytesWritten, NULL)
		&& (dwBytesWritten == dataSEMPQ.cbSize))
		bRetVal = TRUE;	// Success
	else
	{
		sprintf(lpszErrorMessage, "Unable to write to file: %s", lpszEXEName);
	}

	CloseHandle(hSEMPQ);

	return bRetVal;
}

BOOL SEMPQCreator::WritePluginsToSEMPQ(
	LPCSTR lpszEXEName,
	const MPQDRAFTPLUGINMODULE* lpPluginModules,
	DWORD nNumPluginModules,
	SEMPQProgressCallback progressCallback,
	LPVOID lpUserData,
	SEMPQCancellationCheck cancellationCheck,
	LPSTR lpszErrorMessage)
{
	ReportProgress(WRITE_PLUGINS_INITIAL_PROGRESS, "Writing Plugins...", progressCallback, lpUserData);

	// This is pretty straightforward: open, write the modules, close
	EFSHANDLEFORWRITE hEFSFile = OpenEFSFileForWrite(lpszEXEName, 0);
	if (!hEFSFile)
	{
		strcpy(lpszErrorMessage, "Unable to open EFS file for writing");
		return FALSE;
	}

	BOOL bRetVal = FALSE, bCancel = FALSE;
	DWORD iCurModule;
	for (iCurModule = 0; iCurModule < nNumPluginModules; iCurModule++)
	{
		const MPQDRAFTPLUGINMODULE& module = lpPluginModules[iCurModule];
		if (!AddToEFSFile(hEFSFile, module.szModuleFileName,
			module.dwComponentID,
			module.dwModuleID,
			module.bExecute, 0))
			break;

		// Update the progress bar
		int progress = (int)(((float)iCurModule
			* (float)WRITE_PLUGINS_PROGRESS_SIZE / (float)nNumPluginModules)
			+ (float)WRITE_PLUGINS_INITIAL_PROGRESS);
		ReportProgress(progress, "Writing Plugins...", progressCallback, lpUserData);

		// Check for cancellation
		if (CheckCancellation(cancellationCheck, lpUserData))
		{
			bCancel = TRUE;
			break;	// Abort
		}
	}

	// Success is whether we were able to write all modules
	if (iCurModule == nNumPluginModules) {
		bRetVal = TRUE;
	} else if (!bCancel) {
		sprintf(lpszErrorMessage, "Unable to write plugins to file: %s", lpszEXEName);
	} else {
		strcpy(lpszErrorMessage, "Operation cancelled by user");
	}

	CloseEFSFileForWrite(hEFSFile);

	return bRetVal;
}

BOOL SEMPQCreator::WriteMPQToSEMPQ(
	LPCSTR lpszEXEName,
	LPCSTR lpszMPQName,
	SEMPQProgressCallback progressCallback,
	LPVOID lpUserData,
	SEMPQCancellationCheck cancellationCheck,
	LPSTR lpszErrorMessage)
{
	// This is a trivial function that simply opens both files and passes
	// things on to the function that actually does the copying. There is no
	// strategic reason for this division, only an aesthetic one: the code
	// becomes really ugly if we do all of this in one function.
	HANDLE hSEMPQ = CreateFile(lpszEXEName,
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSEMPQ == INVALID_HANDLE_VALUE)
	{
		sprintf(lpszErrorMessage, "Unable to create file: %s", lpszEXEName);
		return FALSE;
	}

	BOOL bRetVal = FALSE;
	HANDLE hMPQ = CreateFile(lpszMPQName, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hMPQ != INVALID_HANDLE_VALUE) {
		bRetVal = WriteMPQToSEMPQ(lpszMPQName, lpszEXEName, hSEMPQ, hMPQ,
			progressCallback, lpUserData, cancellationCheck, lpszErrorMessage);

		CloseHandle(hMPQ);
	} else {
		sprintf(lpszErrorMessage, "Unable to open MPQ: %s", lpszMPQName);
	}

	CloseHandle(hSEMPQ);

	return bRetVal;
}

BOOL SEMPQCreator::WriteMPQToSEMPQ(
	LPCSTR lpszMPQName,
	LPCSTR lpszEXEName,
	HANDLE hSEMPQ,
	HANDLE hMPQ,
	SEMPQProgressCallback progressCallback,
	LPVOID lpUserData,
	SEMPQCancellationCheck cancellationCheck,
	LPSTR lpszErrorMessage)
{
	if (hSEMPQ == INVALID_HANDLE_VALUE || hMPQ == INVALID_HANDLE_VALUE)
	{
		strcpy(lpszErrorMessage, "Invalid file handles");
		return FALSE;
	}

	ReportProgress(WRITE_MPQ_INITIAL_PROGRESS, "Writing MPQ Data...", progressCallback, lpUserData);

	DWORD dwMPQOffset = GetFileSize(hSEMPQ, NULL),
		dwMPQSize = GetFileSize(hMPQ, NULL),
		dwRemaining = dwMPQSize, dwTransferred = 0;

	// Storm searches for MPQs in a file one sector (512 bytes) at a time, so
	// our archive must be written on a sector boundary. Under anything but
	// FUBAR conditions, this condition should automatically be met, as
	// executables must have sizes that are multiples of either 512 or 4096
	// bytes, and the EFS code is also smart enough to ensure this.
	if ((dwMPQOffset % 512) != 0)
	{
		strcpy(lpszErrorMessage, "Internal error: MPQ offset is not sector-aligned");
		return FALSE;
	}

	// 96 is the size of an empty MPQ with a 4-entry hash table (I can't
	// recall if the minimum hash table size is 4 or 16, off the top of my
	// head.
	if (dwMPQSize < 96)
	{
		sprintf(lpszErrorMessage, "Invalid MPQ file (too small): %s", lpszMPQName);
		return FALSE;
	}

	// Allocate the read buffer
	const DWORD dwMaxBufferSize = 256 * 1024;
	DWORD dwBufferSize = (dwRemaining < dwMaxBufferSize) ? dwRemaining : dwMaxBufferSize;

	LPBYTE lpbyReadBuffer = NULL;
	try
	{ lpbyReadBuffer = new BYTE[dwBufferSize]; }
	catch (...)
	{ }

	if (!lpbyReadBuffer)
	{
		sprintf(lpszErrorMessage, "Unable to allocate memory (%u bytes)", dwBufferSize);
		return FALSE;
	}

	// This is a very simple pump in, pump out function: we read in a buffer
	// full from the MPQ, write it out to the SEMPQ.
	SetFilePointer(hSEMPQ, 0, NULL, FILE_END);
	SetFilePointer(hMPQ, 0, NULL, FILE_BEGIN);

	BOOL bRetVal = FALSE, bCancel = FALSE;
	while (dwRemaining)
	{
		DWORD dwBlockSize = (dwRemaining < dwBufferSize) ? dwRemaining : dwBufferSize;
		DWORD dwBytesRead;

		// In and out
		if (!ReadFile(hMPQ, lpbyReadBuffer, dwBlockSize, &dwBytesRead, NULL)
			|| (dwBytesRead < dwBlockSize)
			|| !WriteFile(hSEMPQ, lpbyReadBuffer, dwBlockSize, &dwBytesRead, NULL)
			|| (dwBytesRead < dwBlockSize))
			break;

		// Check for cancellation
		if (CheckCancellation(cancellationCheck, lpUserData))
		{
			bCancel = TRUE;
			break;
		}

		dwTransferred += dwBlockSize;
		dwRemaining -= dwBlockSize;

		// Finally, update the progress bar
		int progress = (int)(((float)dwTransferred
			* WRITE_MPQ_PROGRESS_SIZE
			/ (float)dwMPQSize) + WRITE_MPQ_INITIAL_PROGRESS);
		ReportProgress(progress, "Writing MPQ Data...", progressCallback, lpUserData);
	}

	// Did we finish, or was there an error?
	if (!dwRemaining) {
		SetEndOfFile(hSEMPQ);
		bRetVal = TRUE;
	} else if (!bCancel) {
		sprintf(lpszErrorMessage, "Unable to write to file: %s", lpszEXEName);
	} else {
		strcpy(lpszErrorMessage, "Operation cancelled by user");
	}

	delete [] lpbyReadBuffer;

	if (bRetVal)
		ReportProgress(WRITE_FINISHED, "Writing MPQ Data...", progressCallback, lpUserData);

	return bRetVal;
}

STUBDATA* SEMPQCreator::CreateStubData(
	LPCSTR lpszCustomName,
	LPCSTR lpszRegistryKey,
	LPCSTR lpszRegistryValue,
	LPCSTR lpszProgramPath,
	LPCSTR lpszTargetFileName,
	LPCSTR lpszSpawnFileName,
	LPCSTR lpszParameters,
	int nShuntCount,
	DWORD dwFlags,
	LPSTR lpszErrorMessage)
{
	// Validate parameters
	if (!lpszCustomName || !lpszErrorMessage)
	{
		if (lpszErrorMessage)
			strcpy(lpszErrorMessage, "Invalid parameters: lpszCustomName and lpszErrorMessage are required");
		return NULL;
	}

	// Determine if we're using registry (built-in game) or custom path
	BOOL bUseRegistry = (lpszRegistryKey != NULL && lpszRegistryValue != NULL);
	BOOL bUseCustomPath = (lpszProgramPath != NULL && *lpszProgramPath != '\0');

	if (!bUseRegistry && !bUseCustomPath)
	{
		strcpy(lpszErrorMessage, "Either registry parameters or program path must be specified");
		return NULL;
	}

	if (bUseRegistry && bUseCustomPath)
	{
		strcpy(lpszErrorMessage, "Cannot specify both registry parameters and program path");
		return NULL;
	}

	// Compute the size for the stub data. We need this for both verifying
	// that it will all fit, and so we can get all the offsets correct.
	DWORD cbRegKeyName = 0, cbRegValueName = 0,
		cbTargetPathName = 0, cbTargetFileName = 0,
		cbSpawnFileName = 0, cbArgs = strlen(lpszParameters) + 1;
	char szTargetPath[MAX_PATH] = {0};
	LPCSTR lpszTargetFileNameToUse = NULL;

	// Are we using a supported app, or a custom one?
	if (bUseRegistry)
	{
		// A built-in one
		if (!lpszTargetFileName || !lpszSpawnFileName)
		{
			strcpy(lpszErrorMessage, "Target filename and spawn filename are required for registry-based games");
			return NULL;
		}

		cbRegKeyName = strlen(lpszRegistryKey) + 1;
		cbRegValueName = strlen(lpszRegistryValue) + 1;

		cbTargetFileName = strlen(lpszTargetFileName) + 1;
		cbSpawnFileName = strlen(lpszSpawnFileName) + 1;
	}
	else
	{
		// Custom one
		// Create the target path
		strcpy(szTargetPath, lpszProgramPath);
		PathRemoveFileSpec(szTargetPath);
		lpszTargetFileNameToUse = PathFindFileName(lpszProgramPath);

		if (!lpszTargetFileNameToUse || !*lpszTargetFileNameToUse)
		{
			strcpy(lpszErrorMessage, "Invalid program path: cannot extract filename");
			return NULL;
		}

		cbTargetPathName = strlen(szTargetPath) + 1;
		cbTargetFileName = cbSpawnFileName = strlen(lpszTargetFileNameToUse) + 1;
	}

	// Compute the offsets of the strings
	DWORD nRegKeyOffset = sizeof(PATCHTARGETEX),
		nRegValueOffset = nRegKeyOffset + cbRegKeyName,
		nTargetPathOffset = nRegValueOffset + cbRegValueName,
		nTargetFileOffset = nTargetPathOffset + cbTargetPathName,
		nSpawnFileOffset = nTargetFileOffset + cbTargetFileName,
		nArgsOffset = nSpawnFileOffset + cbSpawnFileName,
		nStubSize = sizeof(STUBDATA) + nArgsOffset + cbArgs - nRegKeyOffset;

	// It shouldn't be possible for this to happen, but who knows
	if (nStubSize > STUBDATASIZE)
	{
		sprintf(lpszErrorMessage, "Stub data size (%u) exceeds maximum (%u)", nStubSize, STUBDATASIZE);
		return NULL;
	}

	// Allocate space for all the data
	STUBDATA* pDataSEMPQ = (STUBDATA*)new BYTE[nStubSize];
	if (!pDataSEMPQ)
	{
		sprintf(lpszErrorMessage, "Unable to allocate memory (%u bytes)", nStubSize);
		return NULL;
	}

	// Set up the basic stub data fields
	pDataSEMPQ->dwDummy = GetTickCount();
	pDataSEMPQ->cbSize = nStubSize;
	pDataSEMPQ->patchTarget.grfFlags = dwFlags;
	strncpy(pDataSEMPQ->szCustomName, lpszCustomName, sizeof(pDataSEMPQ->szCustomName) - 1);
	pDataSEMPQ->szCustomName[sizeof(pDataSEMPQ->szCustomName) - 1] = '\0';

	// Set up the string pointers for the patch target
	PATCHTARGETEX& patchTarget = pDataSEMPQ->patchTarget;

	patchTarget.lpszRegistryKey = (LPCSTR)nRegKeyOffset;
	patchTarget.lpszRegistryValue = (LPCSTR)nRegValueOffset;

	patchTarget.lpszTargetPath = (LPCSTR)nTargetPathOffset;
	patchTarget.lpszTargetFileName = (LPCSTR)nTargetFileOffset;
	patchTarget.lpszSpawnFileName = (LPCSTR)nSpawnFileOffset;

	patchTarget.lpszArguments = (LPCSTR)nArgsOffset;

	// Actually create the patch target strings and other data
	if (bUseRegistry)
	{
		pDataSEMPQ->patchTarget.bUseRegistry = TRUE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nRegKeyOffset,
			lpszRegistryKey);
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nRegValueOffset,
			lpszRegistryValue);
		pDataSEMPQ->patchTarget.bValueIsFileName = FALSE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nSpawnFileOffset,
			lpszSpawnFileName);
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetFileOffset,
			lpszTargetFileName);

		pDataSEMPQ->patchTarget.nShuntCount = nShuntCount;
	}
	else
	{
		pDataSEMPQ->patchTarget.bUseRegistry = FALSE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetPathOffset,
			szTargetPath);

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetFileOffset,
			lpszTargetFileNameToUse);
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nSpawnFileOffset,
			lpszTargetFileNameToUse);

		pDataSEMPQ->patchTarget.nShuntCount = 0;
	}

	strcpy((LPSTR)&pDataSEMPQ->patchTarget + nArgsOffset, lpszParameters);

	return pDataSEMPQ;
}

void SEMPQCreator::ReportProgress(int progress, LPCSTR lpszMessage, SEMPQProgressCallback callback, LPVOID lpUserData)
{
	if (callback)
		callback(progress, lpszMessage, lpUserData);
}

BOOL SEMPQCreator::CheckCancellation(SEMPQCancellationCheck callback, LPVOID lpUserData)
{
	if (callback)
		return callback(lpUserData);
	return FALSE;
}
