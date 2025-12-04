/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

/*
	Shared resource IDs for MPQDraft

	These are Win32 resource IDs for embedded binaries that are used by
	multiple components (CLI, GUI). GUI-specific resources (dialogs, bitmaps,
	icons, strings) are defined in gui/resource.h.

	IMPORTANT: If you change these values, also update gui/resource.h to keep
	them in sync (Visual Studio's resource editor manages that file).
*/

#pragma once

// Application icon
#define IDI_MAINICON      1    // Main application icon

// Embedded binaries (Win32 resources)
#define IDR_PATCHERDLL    159  // MPQDraftDLL.dll embedded in MPQDraft.exe
#define IDR_SEMPQSTUB     143  // MPQStub.exe embedded in MPQDraft.exe
