/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// gamedata.h : Game definitions and supported programs
//

#ifndef GAMEDATA_H
#define GAMEDATA_H

#pragma once

#include <windows.h>

// The structures for the two-tier supported program hierarchy. This 
// structure is for individual patch target components.
struct PROGRAMFILEENTRY
{
	// The name displayed in the list view
	LPCSTR szComponentName;

	// The name of the file to launch, located in the game's install directory
	LPCSTR szFileName;
	// The patch target filename, in the game's install directory
	LPCSTR szTargetFileName;
	// The shunt count. See MPQDraftPatcher for more informatin.
	int nShuntCount;

	// The icon resource ID for the list view entry
	DWORD iIcon;

	BOOL bExtendRedir;	// MPQD_EXTENDED_REDIR
	DWORD dwFlags;
};

// And this one is for installed programs, which have one or more supported 
// patch targets.
struct PROGRAMENTRY
{
	// The display name of the program
	LPCSTR szProgramName;

	// The registry key which contains the registry value
	LPCSTR szRegistryKey;
	// The value name which holds the game's install path
	LPCSTR szRegistryValue;

	// The game's icon resource ID
	DWORD iIcon;

	// The list of components. This list is terminated by an entry containing 
	// a NULL szComponentName.
	const PROGRAMFILEENTRY *files;
};

// The list of supported programs
extern const PROGRAMENTRY SupportApps[];

// Individual game file lists
extern const PROGRAMFILEENTRY DiabloFiles[];
extern const PROGRAMFILEENTRY DiabloHellfireFiles[];
extern const PROGRAMFILEENTRY Diablo2Files[];
extern const PROGRAMFILEENTRY StarcraftFiles[];
extern const PROGRAMFILEENTRY Warcraft2Files[];
extern const PROGRAMFILEENTRY Warcraft3Files[];
extern const PROGRAMFILEENTRY LordsOfMagicFiles[];

#endif // GAMEDATA_H
