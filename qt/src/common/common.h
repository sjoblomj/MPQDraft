/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Common.h - Simplified version for Qt GUI
// This is a subset of /src/core/Common.h containing only what the GUI needs

#if !defined(COMMON_H)
#define COMMON_H

#include <windows.h>
#include "mpqdraftplugin.h"

// Component and module IDs for MPQDraft modules
#define MPQDRAFT_COMPONENT 0x2f0b5f48
#define MPQDRAFTDLL_MODULE 0xa0fcc4e7

// Patching flags
#define MPQD_EXTENDED_REDIR 0x10000
#define MPQD_NO_SPAWNING 0x20000
#define MPQD_USE_D2_STORM 0x40000
#define MPQD_DETECT_STORM 0x80000

// Function pointer type for GetMPQDraftPlugin
typedef BOOL (WINAPI *GetMPQDraftPluginPtr)(OUT IMPQDraftPlugin **lppMPQDraftPlugin);

// Function pointer type for the patcher DLL
typedef BOOL (WINAPI *MPQDraftPatcherPtr)(
    IN LPCSTR lpszApplicationName,
    IN OUT LPSTR lpszCommandLine,
    IN OPTIONAL LPSECURITY_ATTRIBUTES lpProcessAttributes,
    IN OPTIONAL LPSECURITY_ATTRIBUTES lpThreadAttributes,
    IN BOOL bInheritHandles,
    IN DWORD dwCreationFlags,
    IN OPTIONAL LPVOID lpszEnvironment,
    IN OPTIONAL LPCSTR lpszCurrentDirectory,
    IN LPSTARTUPINFO lpStartupInfo,
    IN DWORD dwFlags,
    IN LPCSTR lpszMPQDraftDir,
    IN LPCSTR lpszTargetPath,
    IN OPTIONAL DWORD nShuntCount,
    IN OPTIONAL DWORD nPatchMPQs,
    IN OPTIONAL DWORD nAuxModules,
    IN OPTIONAL LPCSTR *lplpszMPQNames, 
    IN OPTIONAL const MPQDRAFTPLUGINMODULE *lpAuxModules
);

#endif // !defined(COMMON_H)

