/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Patching flags and component/module IDs for the MPQDraft system
#if !defined(PATCHERFLAGS_H)
#define PATCHERFLAGS_H

// Component and module IDs for MPQDraft modules
#define MPQDRAFT_COMPONENT 0x2f0b5f48
#define MPQDRAFTDLL_MODULE 0xa0fcc4e7

// Patching flags
// Redirect file open attempts that explicitly specify an archive to open the file in
#define MPQD_EXTENDED_REDIR 0x10000
// Do not inject the MPQDraft system into child processes create by the patch target
#define MPQD_NO_SPAWNING  0x20000
#define MPQD_USE_D2_STORM 0x40000
#define MPQD_DETECT_STORM 0x80000

#endif
