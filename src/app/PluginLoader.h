/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// PluginLoader.h : Shared plugin loading utilities
//

#if !defined(PLUGINLOADER_H)
#define PLUGINLOADER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <string>
#include "../core/Common.h"

/////////////////////////////////////////////////////////////////////////////
/*	PluginInfo
	A simple structure to hold information about a loaded plugin.
	This is a lightweight, non-GUI alternative to CPluginPage::PLUGINENTRY
	that can be used by both CLI and GUI code. */

struct PluginInfo
{
	// The path of the plugin
	std::string strFileName;

	// The plugin's ID from IMPQDraftPlugin
	DWORD dwPluginID;

	// The plugin's name from IMPQDraftPlugin
	std::string strPluginName;

	// The plugin's DLL handle (caller is responsible for freeing)
	HMODULE hDLLModule;

	// The plugin's interface (caller is responsible for cleanup)
	IMPQDraftPlugin *pPlugin;

	PluginInfo()
		: dwPluginID(0)
		, hDLLModule(NULL)
		, pPlugin(NULL)
	{
	}
};

/////////////////////////////////////////////////////////////////////////////
/*	LoadPluginInfo
	Loads a plugin DLL and retrieves its information.

	Parameters:
		lpszFileName [in] - Full path to the plugin DLL
		pluginInfo [out] - Structure to receive plugin information

	Returns:
		TRUE on success, FALSE on failure

	Notes:
		- On success, the caller is responsible for calling FreeLibrary on
		  pluginInfo.hDLLModule when done with the plugin
		- On failure, any resources are cleaned up automatically */

BOOL LoadPluginInfo(IN LPCSTR lpszFileName, OUT PluginInfo &pluginInfo);

#endif // !defined(PLUGINLOADER_H)
