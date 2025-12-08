/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// SEMPQCreator.cpp - SEMPQ creation implementation

#include "SEMPQCreator.h"
#include "../core/MPQDraftPlugin.h"

#ifdef _WIN32
#include "SEMPQData.h"
#include "../core/PatcherFlags.h"
#include "../common/QResource.h"
#include "../app/resource_ids.h"
#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////

static STUBDATA* CreateStubDataFromParams(const SEMPQCreationParams& params, std::string& errorMessage);

/////////////////////////////////////////////////////////////////////////////
// SEMPQCreator implementation
/////////////////////////////////////////////////////////////////////////////

bool SEMPQCreator::createSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
	// Validate parameters
	if (params.outputPath.empty())
	{
		errorMessage = "Output path is empty";
		return false;
	}

	if (params.sempqName.empty())
	{
		errorMessage = "SEMPQ name is empty";
		return false;
	}

	if (params.mpqPath.empty())
	{
		errorMessage = "MPQ path is empty";
		return false;
	}

	// Check if MPQ file exists
	DWORD dwAttrib = GetFileAttributes(params.mpqPath.c_str());
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		errorMessage = "The MPQ file does not exist: " + params.mpqPath;
		return false;
	}

	// Step 1: Write stub to SEMPQ
	if (!writeStubToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
		return false;

	// Step 1.5: Write custom icon to SEMPQ (if specified)
	if (!params.iconPath.empty())
	{
		if (!writeIconToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
			return false;
	}

	// Step 2: Write plugins to SEMPQ
	if (!writePluginsToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
		return false;

	// Step 3: Write MPQ to SEMPQ
	if (!writeMPQToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
		return false;

	// Success!
	if (progressCallback)
		progressCallback(WRITE_FINISHED, "SEMPQ created successfully!");
	return true;
}

// Helper: Get the offset where stub data should be written in the stub executable
static DWORD GetStubDataWriteOffset(const std::string& stubFileName)
{
	// Manually finding the address of the stub data in an executable is to
	// be avoided, where possible. Fortunately, there's a clever way to get
	// this information through Windows: the LOAD_LIBRARY_AS_DATAFILE flag.
	// When this flag is used to load a module, the file is mapped into
	// memory as a single chunk, in exactly the same format as the file
	// itself. Some Windows functions (including the resource functions) are
	// able to operate on a file loaded in this way. In this case, the RVA of
	// the resource corresponds exactly to the file offset.
	if (stubFileName.empty())
		return 0;

	// Load the stub as a data file
	HMODULE hStub = LoadLibraryEx(stubFileName.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
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

bool SEMPQCreator::writeStubToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
	if (progressCallback)
		progressCallback(WRITE_STUB_INITIAL_PROGRESS, "Writing Executable Code...\n");

	// Create the STUBDATA from parameters
	STUBDATA* pStubData = CreateStubDataFromParams(params, errorMessage);
	if (!pStubData)
		return false;

	// We've got a couple tasks to do here. First, we need to create the SEMPQ
	// file and write the unmodified version of the stub.
	if (!ExtractResource(NULL, MAKEINTRESOURCE(IDR_SEMPQSTUB), "EXE", params.outputPath.c_str()))
	{
		errorMessage = "Unable to create file: " + params.outputPath;
		delete [] (BYTE*)pStubData;
		return false;
	}

	// Next, create the new stub data that contains the info for our mod
	DWORD dwStubDataOffset = GetStubDataWriteOffset(params.outputPath);
	if (!dwStubDataOffset)
	{
		errorMessage = "Internal error: unable to locate stub data offset";
		delete [] (BYTE*)pStubData;
		return false;
	}

	// Open the file, and...
	HANDLE hSEMPQ = CreateFile(params.outputPath.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSEMPQ == INVALID_HANDLE_VALUE)
	{
		errorMessage = "Unable to open file: " + params.outputPath;
		delete [] (BYTE*)pStubData;
		return false;
	}

	// Write the stub data
	bool bRetVal = false;
	DWORD dwBytesWritten;
	if (SetFilePointer(hSEMPQ, dwStubDataOffset, NULL, FILE_BEGIN) == dwStubDataOffset
		&& WriteFile(hSEMPQ, pStubData, pStubData->cbSize, &dwBytesWritten, NULL)
		&& (dwBytesWritten == pStubData->cbSize))
		bRetVal = true;	// Success
	else
	{
		errorMessage = "Unable to write to file: " + params.outputPath;
	}

	CloseHandle(hSEMPQ);
	delete [] (BYTE*)pStubData;

	return bRetVal;
}

bool SEMPQCreator::writePluginsToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
	if (progressCallback)
		progressCallback(WRITE_PLUGINS_INITIAL_PROGRESS, "Writing Plugins...\n");

	// First, extract the MPQDraft patcher DLL to a temporary file.
	// This DLL is REQUIRED for the SEMPQ to function - the stub executable
	// loads it to perform the actual patching.
	char szPatcherDLLPath[MAX_PATH + 1];
	if (!ExtractTempResource(NULL, MAKEINTRESOURCE(IDR_PATCHERDLL), "DLL", szPatcherDLLPath))
	{
		errorMessage = "Unable to extract patcher DLL from resources";
		return false;
	}

	// Open the EFS file for writing. We always need to create the EFS file
	// because the patcher DLL must be embedded even if there are no user plugins.
	EFSHANDLEFORWRITE hEFSFile = OpenEFSFileForWrite(params.outputPath.c_str(), 0);
	if (!hEFSFile)
	{
		errorMessage = "Unable to open EFS file for writing";
		return false;
	}

	bool bRetVal = false, bCancel = false;

	// First, add the MPQDraft patcher DLL with the required component/module IDs.
	// The stub executable looks for this specific DLL by these IDs.
	// Note: bExecute (dwData) must be FALSE - the patcher DLL is not a plugin,
	// it's loaded directly by the stub to perform patching.
	if (!AddToEFSFile(hEFSFile, szPatcherDLLPath,
		MPQDRAFT_COMPONENT,
		MPQDRAFTDLL_MODULE,
		FALSE, 0))  // bExecute=FALSE - not a plugin
	{
		errorMessage = "Unable to write patcher DLL to EFS file";
		CloseEFSFileForWrite(hEFSFile);
		return false;
	}

	// Now add any user-specified plugin modules
	size_t iCurModule;
	for (iCurModule = 0; iCurModule < params.pluginModules.size(); iCurModule++)
	{
		const MPQDRAFTPLUGINMODULE& module = params.pluginModules[iCurModule];

		// Use the actual component/module IDs from the plugin module structure
		if (!AddToEFSFile(hEFSFile, module.szModuleFileName,
			module.dwComponentID,
			module.dwModuleID,
			module.bExecute, 0))
			break;

		// Update the progress bar
		int progress = (int)(((float)iCurModule
			* (float)WRITE_PLUGINS_PROGRESS_SIZE / (float)params.pluginModules.size())
			+ (float)WRITE_PLUGINS_INITIAL_PROGRESS);
		if (progressCallback)
			progressCallback(progress, "Writing Plugins...\n");

		// Check for cancellation
		if (cancellationCheck && cancellationCheck())
		{
			bCancel = true;
			break;	// Abort
		}
	}

	// Success is whether we were able to write all modules
	if (iCurModule == params.pluginModules.size()) {
		bRetVal = true;
	} else if (!bCancel) {
		errorMessage = "Unable to write plugins to file: " + params.outputPath;
	} else {
		errorMessage = "Operation cancelled by user";
	}

	CloseEFSFileForWrite(hEFSFile);

	return bRetVal;
}

bool SEMPQCreator::writeMPQToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
	if (progressCallback)
		progressCallback(WRITE_MPQ_INITIAL_PROGRESS, "Writing MPQ Data...\n");

	// Open the SEMPQ file for writing
	HANDLE hSEMPQ = CreateFile(params.outputPath.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSEMPQ == INVALID_HANDLE_VALUE)
	{
		errorMessage = "Unable to open file: " + params.outputPath;
		return false;
	}

	// Open the MPQ file for reading
	HANDLE hMPQ = CreateFile(params.mpqPath.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hMPQ == INVALID_HANDLE_VALUE) {
		errorMessage = "Unable to open MPQ: " + params.mpqPath;
		CloseHandle(hSEMPQ);
		return false;
	}

	// Get file sizes
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
		errorMessage = "Internal error: MPQ offset is not sector-aligned";
		CloseHandle(hMPQ);
		CloseHandle(hSEMPQ);
		return false;
	}

    // 96 is the size of an empty MPQ with a 4-entry hash table (I can't
    // recall if the minimum hash table size is 4 or 16, off the top of my
    // head.
	if (dwMPQSize < 96)
	{
		errorMessage = "Invalid MPQ file (too small): " + params.mpqPath;
		CloseHandle(hMPQ);
		CloseHandle(hSEMPQ);
		return false;
	}

	// Allocate the read buffer
	const DWORD dwMaxBufferSize = 256 * 1024;
	DWORD dwBufferSize = (dwRemaining < dwMaxBufferSize) ? dwRemaining : dwMaxBufferSize;

	LPBYTE lpbyReadBuffer = nullptr;
	try
	{ lpbyReadBuffer = new BYTE[dwBufferSize]; }
	catch (...)
	{ }

	if (!lpbyReadBuffer)
	{
		errorMessage = "Unable to allocate memory (" + std::to_string(dwBufferSize) + " bytes)";
		CloseHandle(hMPQ);
		CloseHandle(hSEMPQ);
		return false;
	}

	// This is a very simple pump in, pump out function: we read in a buffer
	// full from the MPQ, write it out to the SEMPQ.
	SetFilePointer(hSEMPQ, 0, NULL, FILE_END);
	SetFilePointer(hMPQ, 0, NULL, FILE_BEGIN);

	bool bRetVal = false, bCancel = false;
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
		if (cancellationCheck && cancellationCheck())
		{
			bCancel = true;
			break;
		}

		dwTransferred += dwBlockSize;
		dwRemaining -= dwBlockSize;

		// Finally, update the progress bar
		int progress = (int)(((float)dwTransferred
			* WRITE_MPQ_PROGRESS_SIZE
			/ (float)dwMPQSize) + WRITE_MPQ_INITIAL_PROGRESS);
		if (progressCallback)
			progressCallback(progress, "Writing MPQ Data...\n");
	}

	// Did we finish, or was there an error?
	if (!dwRemaining) {
		SetEndOfFile(hSEMPQ);
		bRetVal = true;
	} else if (!bCancel) {
		errorMessage = "Unable to write to file: " + params.outputPath;
	} else {
		errorMessage = "Operation cancelled by user";
	}

	delete [] lpbyReadBuffer;

	if (bRetVal && progressCallback)
		progressCallback(WRITE_FINISHED, "Writing MPQ Data...\n");

	CloseHandle(hMPQ);
	CloseHandle(hSEMPQ);

	return bRetVal;
}

// Helper: Create STUBDATA structure from parameters
static STUBDATA* CreateStubDataFromParams(const SEMPQCreationParams& params, std::string& errorMessage)
{
	// Validate parameters
	if (params.sempqName.empty())
	{
		errorMessage = "Invalid parameters: sempqName is required";
		return nullptr;
	}

	// Determine if we're using registry (built-in game) or custom path
	bool bUseRegistry = params.useRegistry;
	bool bUseCustomPath = !params.targetPath.empty();

	if (!bUseRegistry && !bUseCustomPath)
	{
		errorMessage = "Either registry parameters or target path must be specified";
		return nullptr;
	}

	if (bUseRegistry && bUseCustomPath)
	{
		errorMessage = "Cannot specify both registry parameters and target path";
		return nullptr;
	}

    // Compute the size for the stub data. We need this for both verifying
    // that it will all fit, and so we can get all the offsets correct.
	DWORD cbRegKeyName = 0, cbRegValueName = 0,
			cbTargetPathName = 0, cbTargetFileName = 0,
			cbSpawnFileName = 0, cbArgs = params.parameters.length() + 1;
	char szTargetPath[MAX_PATH] = {0};
	std::string targetFileNameToUse;

	// Are we using a supported app, or a custom one?
	if (bUseRegistry)
	{
		// A built-in one
		if (params.targetFileName.empty() || params.spawnFileName.empty())
		{
			errorMessage = "Target filename and spawn filename are required for registry-based games";
			return nullptr;
		}

		cbRegKeyName     = params.registryKey   .length() + 1;
		cbRegValueName   = params.registryValue .length() + 1;

		cbTargetFileName = params.targetFileName.length() + 1;
		cbSpawnFileName  = params.spawnFileName .length() + 1;
	}
	else
	{
		// Custom one
		// Create the target path
		strcpy(szTargetPath, params.targetPath.c_str());
		PathRemoveFileSpec(szTargetPath);
		const char* lpszFileName = PathFindFileName(params.targetPath.c_str());

		if (!lpszFileName || !*lpszFileName)
		{
			errorMessage = "Invalid target path: cannot extract filename";
			return nullptr;
		}

		targetFileNameToUse = lpszFileName;
		cbTargetPathName = strlen(szTargetPath) + 1;
		cbTargetFileName = cbSpawnFileName = targetFileNameToUse.length() + 1;
	}

	// Compute the offsets of the strings
	DWORD nRegKeyOffset = sizeof(PATCHTARGETEX),
			nRegValueOffset   = nRegKeyOffset     + cbRegKeyName,
			nTargetPathOffset = nRegValueOffset   + cbRegValueName,
			nTargetFileOffset = nTargetPathOffset + cbTargetPathName,
			nSpawnFileOffset  = nTargetFileOffset + cbTargetFileName,
			nArgsOffset       = nSpawnFileOffset  + cbSpawnFileName,
			nStubSize = sizeof(STUBDATA) + nArgsOffset + cbArgs - nRegKeyOffset;

	// It shouldn't be possible for this to happen, but who knows
	if (nStubSize > STUBDATASIZE)
	{
		errorMessage = "Stub data size (" + std::to_string(nStubSize) + ") exceeds maximum (" + std::to_string(STUBDATASIZE) + ")";
		return nullptr;
	}

	// Allocate space for all the data
	STUBDATA* pDataSEMPQ = (STUBDATA*)new BYTE[nStubSize];
	if (!pDataSEMPQ)
	{
		errorMessage = "Unable to allocate memory (" + std::to_string(nStubSize) + " bytes)";
		return nullptr;
	}

	// Set up the basic stub data fields
	pDataSEMPQ->dwDummy = GetTickCount();
	pDataSEMPQ->cbSize = nStubSize;
	pDataSEMPQ->patchTarget.grfFlags = params.flags;
	strncpy(pDataSEMPQ->szCustomName, params.sempqName.c_str(), sizeof(pDataSEMPQ->szCustomName) - 1);
	pDataSEMPQ->szCustomName[sizeof(pDataSEMPQ->szCustomName) - 1] = '\0';

	// Set up the string pointers for the patch target
	PATCHTARGETEX& patchTarget = pDataSEMPQ->patchTarget;

	patchTarget.lpszRegistryKey    = (LPCSTR)nRegKeyOffset;
	patchTarget.lpszRegistryValue  = (LPCSTR)nRegValueOffset;

	patchTarget.lpszTargetPath     = (LPCSTR)nTargetPathOffset;
	patchTarget.lpszTargetFileName = (LPCSTR)nTargetFileOffset;
	patchTarget.lpszSpawnFileName  = (LPCSTR)nSpawnFileOffset;

	patchTarget.lpszArguments = (LPCSTR)nArgsOffset;

	// Actually create the patch target strings and other data
	if (bUseRegistry)
	{
		pDataSEMPQ->patchTarget.bUseRegistry = TRUE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nRegKeyOffset,     params.registryKey.c_str());
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nRegValueOffset,   params.registryValue.c_str());
		pDataSEMPQ->patchTarget.bValueIsFileName = params.valueIsFullPath ? TRUE : FALSE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nSpawnFileOffset,  params.spawnFileName.c_str());
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetFileOffset, params.targetFileName.c_str());

		pDataSEMPQ->patchTarget.nShuntCount = params.shuntCount;
	}
	else
	{
		pDataSEMPQ->patchTarget.bUseRegistry = FALSE;

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetPathOffset, szTargetPath);

		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nTargetFileOffset, targetFileNameToUse.c_str());
		strcpy((LPSTR)&pDataSEMPQ->patchTarget + nSpawnFileOffset,  targetFileNameToUse.c_str());

		pDataSEMPQ->patchTarget.nShuntCount = 0;
	}

	strcpy((LPSTR)&pDataSEMPQ->patchTarget + nArgsOffset, params.parameters.c_str());

	return pDataSEMPQ;
}

/////////////////////////////////////////////////////////////////////////////
// Icon file structures (for reading .ico files)
/////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

// Icon file header
typedef struct {
	WORD idReserved;   // Reserved (must be 0)
	WORD idType;       // Resource type (1 for icons)
	WORD idCount;      // Number of images
} ICONDIR;

// Icon directory entry in .ico file
typedef struct {
	BYTE  bWidth;          // Width of image (0 means 256)
	BYTE  bHeight;         // Height of image (0 means 256)
	BYTE  bColorCount;     // Number of colors (0 if >= 8bpp)
	BYTE  bReserved;       // Reserved (must be 0)
	WORD  wPlanes;         // Color planes
	WORD  wBitCount;       // Bits per pixel
	DWORD dwBytesInRes;    // Size of image data
	DWORD dwImageOffset;   // Offset to image data in file
} ICONDIRENTRY;

// Icon directory entry in RT_GROUP_ICON resource
typedef struct {
	BYTE  bWidth;
	BYTE  bHeight;
	BYTE  bColorCount;
	BYTE  bReserved;
	WORD  wPlanes;
	WORD  wBitCount;
	DWORD dwBytesInRes;
	WORD  nId;             // Resource ID of the icon
} GRPICONDIRENTRY;

// Group icon directory header (for RT_GROUP_ICON resource)
typedef struct {
	WORD idReserved;
	WORD idType;
	WORD idCount;
	// GRPICONDIRENTRY entries follow
} GRPICONDIR;

#pragma pack(pop)

bool SEMPQCreator::writeIconToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
	if (cancellationCheck && cancellationCheck())
	{
		errorMessage = "Operation cancelled";
		return false;
	}

	if (progressCallback)
		progressCallback(WRITE_STUB_INITIAL_PROGRESS + 2, "Writing Custom Icon...\n");

	// Check if icon file exists
	DWORD dwAttrib = GetFileAttributes(params.iconPath.c_str());
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		errorMessage = "The icon file does not exist: " + params.iconPath;
		return false;
	}

	// Open and read the icon file
	HANDLE hIconFile = CreateFile(params.iconPath.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hIconFile == INVALID_HANDLE_VALUE)
	{
		errorMessage = "Unable to open icon file: " + params.iconPath;
		return false;
	}

	DWORD dwIconFileSize = GetFileSize(hIconFile, NULL);
	if (dwIconFileSize < sizeof(ICONDIR))
	{
		CloseHandle(hIconFile);
		errorMessage = "Invalid icon file (too small): " + params.iconPath;
		return false;
	}

	// Read the entire icon file into memory
	LPBYTE lpIconData = new BYTE[dwIconFileSize];
	DWORD dwBytesRead;
	if (!ReadFile(hIconFile, lpIconData, dwIconFileSize, &dwBytesRead, NULL) ||
		dwBytesRead != dwIconFileSize)
	{
		delete[] lpIconData;
		CloseHandle(hIconFile);
		errorMessage = "Unable to read icon file: " + params.iconPath;
		return false;
	}
	CloseHandle(hIconFile);

	// Parse the icon file header
	ICONDIR* pIconDir = (ICONDIR*)lpIconData;
	if (pIconDir->idReserved != 0 || pIconDir->idType != 1 || pIconDir->idCount == 0)
	{
		delete[] lpIconData;
		errorMessage = "Invalid icon file format: " + params.iconPath;
		return false;
	}

	WORD nIconCount = pIconDir->idCount;
	ICONDIRENTRY* pIconEntries = (ICONDIRENTRY*)(lpIconData + sizeof(ICONDIR));

	// Begin updating resources in the SEMPQ file
	HANDLE hUpdate = BeginUpdateResource(params.outputPath.c_str(), FALSE);
	if (hUpdate == NULL)
	{
		delete[] lpIconData;
		errorMessage = "Unable to begin resource update on: " + params.outputPath;
		return false;
	}

	bool bSuccess = true;

	// Create the GRPICONDIR structure for RT_GROUP_ICON
	// Size = header + entries
	DWORD dwGrpIconSize = sizeof(GRPICONDIR) + nIconCount * sizeof(GRPICONDIRENTRY);
	LPBYTE lpGrpIconData = new BYTE[dwGrpIconSize];
	GRPICONDIR* pGrpIconDir = (GRPICONDIR*)lpGrpIconData;
	pGrpIconDir->idReserved = 0;
	pGrpIconDir->idType = 1;
	pGrpIconDir->idCount = nIconCount;

	GRPICONDIRENTRY* pGrpEntries = (GRPICONDIRENTRY*)(lpGrpIconData + sizeof(GRPICONDIR));

	// Add each icon image as a separate RT_ICON resource
	// Use resource IDs starting from 1
	for (WORD i = 0; i < nIconCount && bSuccess; i++)
	{
		ICONDIRENTRY* pEntry = &pIconEntries[i];
		WORD nIconId = i + 1;  // Resource IDs start at 1

		// Copy the icon image data
		LPBYTE lpImageData = lpIconData + pEntry->dwImageOffset;
		DWORD dwImageSize = pEntry->dwBytesInRes;

		// Update the RT_ICON resource
		if (!UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(nIconId),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			lpImageData, dwImageSize))
		{
			bSuccess = false;
			errorMessage = "Unable to update icon resource #" + std::to_string(nIconId);
		}

		// Fill in the GRPICONDIRENTRY
		pGrpEntries[i].bWidth = pEntry->bWidth;
		pGrpEntries[i].bHeight = pEntry->bHeight;
		pGrpEntries[i].bColorCount = pEntry->bColorCount;
		pGrpEntries[i].bReserved = pEntry->bReserved;
		pGrpEntries[i].wPlanes = pEntry->wPlanes;
		pGrpEntries[i].wBitCount = pEntry->wBitCount;
		pGrpEntries[i].dwBytesInRes = pEntry->dwBytesInRes;
		pGrpEntries[i].nId = nIconId;
	}

	// Update the RT_GROUP_ICON resource
	// The stub uses IDI_MAINICON (105) as the icon resource ID
	if (bSuccess)
	{
		// First, try to update the existing group icon (ID 105 = IDI_MAINICON from stub/resource.h)
		// We use ID 1 as that's typically the main icon in Windows executables
		if (!UpdateResource(hUpdate, RT_GROUP_ICON, MAKEINTRESOURCE(1),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			lpGrpIconData, dwGrpIconSize))
		{
			// If ID 1 doesn't exist, try the stub's icon ID (105)
			if (!UpdateResource(hUpdate, RT_GROUP_ICON, MAKEINTRESOURCE(105),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				lpGrpIconData, dwGrpIconSize))
			{
				bSuccess = false;
				errorMessage = "Unable to update group icon resource";
			}
		}
	}

	delete[] lpGrpIconData;
	delete[] lpIconData;

	// Commit the resource updates
	if (!EndUpdateResource(hUpdate, !bSuccess))
	{
		if (bSuccess)
		{
			errorMessage = "Unable to commit resource updates to: " + params.outputPath;
			bSuccess = false;
		}
	}

	return bSuccess;
}

#else

// non-Windows stub implementation
// SEMPQ creation is Windows-only functionality

bool SEMPQCreator::createSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
    (void)params;            // Suppress unused parameter warning
    (void)progressCallback;  // Suppress unused parameter warning
    (void)cancellationCheck; // Suppress unused parameter warning
	errorMessage = "SEMPQ creation is only supported on Windows";
	return false;
}

bool SEMPQCreator::writeStubToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
    (void)params;            // Suppress unused parameter warning
    (void)progressCallback;  // Suppress unused parameter warning
    (void)cancellationCheck; // Suppress unused parameter warning
	errorMessage = "SEMPQ creation is only supported on Windows";
	return false;
}

bool SEMPQCreator::writePluginsToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
    (void)params;            // Suppress unused parameter warning
    (void)progressCallback;  // Suppress unused parameter warning
    (void)cancellationCheck; // Suppress unused parameter warning
	errorMessage = "SEMPQ creation is only supported on Windows";
	return false;
}

bool SEMPQCreator::writeMPQToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
    (void)params;            // Suppress unused parameter warning
    (void)progressCallback;  // Suppress unused parameter warning
    (void)cancellationCheck; // Suppress unused parameter warning
	errorMessage = "SEMPQ creation is only supported on Windows";
	return false;
}

bool SEMPQCreator::writeIconToSEMPQ(
	const SEMPQCreationParams& params,
	ProgressCallback progressCallback,
	CancellationCheck cancellationCheck,
	std::string& errorMessage)
{
    (void)params;            // Suppress unused parameter warning
    (void)progressCallback;  // Suppress unused parameter warning
    (void)cancellationCheck; // Suppress unused parameter warning
	errorMessage = "SEMPQ creation is only supported on Windows";
	return false;
}

#endif // _WIN32
