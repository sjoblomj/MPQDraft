/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Structures and constants related to SEMPQ creation and execution
#if !defined(SEMPQDATA_H)
#define SEMPQDATA_H

#include <windows.h>
#include "PatcherFlags.h"

// SEMPQs are fairly complicated. They consist of, in order, the SEMPQ executable stub, and Embedded File System (EFS), and the MPQ itself. Apart from the STUBDATA resource, the SEMPQ portion is invariant. The EFS contains all the plugins and plugin support files, as well as the MPQDraft DLL itself. The MPQ is just the MPQ used when creating the SEMPQ; originally, Storm would not load MPQs which were not at the very end of the disk file, but that restriction has since been removed to support the strong digital signature in Warcraft III.
#define STUBDATASIZE 0x400
#define STUBDATA_KEY 0xD7DCA2D6 // STUBDATA in MPQ hash

// The description of the patch target which is stored in an SEMPQ's STUBDATA structure. Note that all pointers are relative offsets from the start of the PATCHTARGETEX when stored in the SEMPQ, and adjusted when loaded into memory. For the exact meaning of fields, see MPQStub.cpp.
struct PATCHTARGETEX
{
	BOOL bUseRegistry;	// If FALSE, use file path directly

	// Registry key and value name to locate the patch target's directory
	LPCSTR lpszRegistryKey;
	LPCSTR lpszRegistryValue;
	BOOL bValueIsFileName;	// If TRUE, registry value is treated as a full path, else a directory

	LPCSTR lpszTargetPath;	// Directory of patch target; used if bUseRegistry is FALSE
	// File that will be patached; ALWAYS used. If null, target is same as spawn file spec.
	LPCSTR lpszTargetFileName;

	// The filename to combine with the directory in the registry when launching the patch target (may not be the patch target itself). Used if bValueIsFileName is FALSE.
	LPCSTR lpszSpawnFileName;
	DWORD nShuntCount;

	LPCSTR lpszArguments;

	DWORD iIcon;	// Icon ID in internal arrays

	DWORD grfFlags;	// Flags sent to patcher
};

// This STUBDATA is a resource in the SEMPQ used to store info about the patching operation to be performed. This data is overwritten when creating the SEMPQ. Perhaps at some point this should be changed to be a data file embedded in the SEMPQ's EFS, as that would be easier than overwriting the resource. There was a reason for originally using a resource, but it's become obsolete (rather, it was an idea for a feature in the future, which was never actually implemented and ultimately abandoned).
#include <pshpack1.h>
struct STUBDATA
{
	DWORD dwDummy;	// Don't ask what this is; it's a long story

	DWORD cbSize;	// sizeof(STUBDATA)

	// The name of the SEMPQ. Used in error messages and things.
	char szCustomName[32];

	PATCHTARGETEX patchTarget;

	// The actual strings used in PATCHTARGETEX will reside here
};
#include <poppack.h>

#endif
