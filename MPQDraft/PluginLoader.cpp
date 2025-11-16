/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// PluginLoader.cpp : Implementation of shared plugin loading utilities
//

#include "stdafx.h"
#include "PluginLoader.h"

BOOL LoadPluginInfo(IN LPCSTR lpszFileName, OUT PluginInfo &pluginInfo)
{
	// Load the plugin's module
	pluginInfo.hDLLModule = LoadLibrary(lpszFileName);
	if (!pluginInfo.hDLLModule)
		return FALSE;

	// Get the plugin's interface
	GetMPQDraftPluginPtr pGetMPQDraftPlugin = (GetMPQDraftPluginPtr)
		GetProcAddress(pluginInfo.hDLLModule, "GetMPQDraftPlugin");

	if (!pGetMPQDraftPlugin || !pGetMPQDraftPlugin(&pluginInfo.pPlugin))
	{
		FreeLibrary(pluginInfo.hDLLModule);
		pluginInfo.hDLLModule = NULL;
		return FALSE;
	}

	// Get the plugin's ID
	pluginInfo.pPlugin->Identify(&pluginInfo.dwPluginID);

	// Get the plugin's name
	char szPluginName[255];
	pluginInfo.pPlugin->GetPluginName(szPluginName, sizeof(szPluginName));
	pluginInfo.strPluginName = szPluginName;

	// Store the filename
	pluginInfo.strFileName = lpszFileName;

	return TRUE;
}
