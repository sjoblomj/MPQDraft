/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Function pointer types for dynamically loading the MPQDraft patcher DLL
#if !defined(PATCHERAPI_H)
#define PATCHERAPI_H

#include <windows.h>
#include "PatcherFlags.h"
#include "MPQDraftPlugin.h"

/* 
	* CreateAndPatchProcess *
	This is the function that interfaces the high level (the GUI or the SEMPQ)
    with the low level (the patcher system and patcher DLL). Specifically,
    this is the only portion of the patching kernel that is directly exposed
    to the GUI and SEMPQ; all the stuff below this is internal to the DLL.
    This function itself is in the DLL, and must be called through a pointer
    obtained with GetProcAddress.

    Creates the process according the specs provided in the NEWPROCESSINFO,
    then performs injection of the MPQDraft system, and returns output info
    to the caller in NEWPROCESSINFO. Any cleanup of the NEWPROCESSINFO
    (e.g. the handles in lpProcessInformation) must be done by the caller.
    If FALSE is returned, the patching was unsuccessful, and the process
    should be terminated.
*/
typedef BOOL (WINAPI *MPQDraftPatcherPtr)(
	// These first 10 parameters are nothing other than the parameters passed to CreateProcess, and used for just that.
	IN LPCSTR lpszApplicationName,
	IN OUT LPSTR lpszCommandLine,

	IN OPTIONAL LPSECURITY_ATTRIBUTES lpProcessAttributes,
	IN OPTIONAL LPSECURITY_ATTRIBUTES lpThreadAttributes,

	IN BOOL bInheritHandles,
	IN DWORD dwCreationFlags,
	IN OPTIONAL LPVOID lpszEnvironment,
	IN OPTIONAL LPCSTR lpszCurrentDirectory,

	IN LPSTARTUPINFO lpStartupInfo,
	// Patching flags, which indicate various options for the patching operation
	IN DWORD dwFlags,
	// The directory of the MPQDraft program or the SEMPQ
	IN LPCSTR lpszMPQDraftDir,
	// The path of the patch target. The DLL will be injected into all processes, but only this process will have the MPQs and plugins loaded.
	IN LPCSTR lpszTargetPath,
	// Number of times to execute the patch target before actually loading the MPQs and plugins (see PATCHTARGETEX)
	IN OPTIONAL DWORD nShuntCount,
	// The number of MPQs to load in the target process
	IN OPTIONAL DWORD nPatchMPQs,
	// The number of auxiliary modules for the patching operation. Any modules marked as executable will be loaded as plugins.
	IN OPTIONAL DWORD nAuxModules,
	// The list of MPQ paths, one for each MPQ to be loaded. Later entries have higher priority.
	IN OPTIONAL LPCSTR *lplpszMPQNames,
	// The list of auxiliary modules. Plugins will be loaded in order.
	IN OPTIONAL const MPQDRAFTPLUGINMODULE *lpAuxModules
);

typedef BOOL (WINAPI *GetMPQDraftPluginPtr)(OUT IMPQDraftPlugin **lppMPQDraftPlugin);

#endif
