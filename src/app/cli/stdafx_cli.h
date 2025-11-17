/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// stdafx_cli.h : MFC-free precompiled header for CLI components
//
// This header is used by CLI code to avoid MFC dependencies.
// It includes only standard Windows headers and STL.

#pragma once

// Target Windows XP and later
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

// Windows headers
#include <windows.h>
#include <shlwapi.h>

// Standard C/C++ headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STL headers
#include <string>
#include <vector>
#include <exception>

// Debug macros (MFC-free versions)
#ifdef _DEBUG
	#define ASSERT(x) if (!(x)) { __debugbreak(); }
	#define VERIFY(x) ASSERT(x)
#else
	#define ASSERT(x) ((void)0)
	#define VERIFY(x) (x)
#endif

// Common MPQDraft headers
#include <QDebug.h>
#include <QInjectDLL.h>
#include <QResource.h>
