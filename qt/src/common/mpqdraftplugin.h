/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

/*
    MPQDraftPlugin.h
    The central hive of the MPQDraft plugin system.
    
    This is a copy from /src/core/MPQDraftPlugin.h for the Qt GUI.
    See the original file for full documentation.
*/

#ifndef QDPLUGIN_H
#define QDPLUGIN_H

#include <windows.h>

// The maximum length of a plugin module's filename. INCLUDES final NULL.
#define MPQDRAFT_MAX_PATH 264

// The maximum length of a plugin's name. INCLUDES final NULL.
#define MPQDRAFT_MAX_PLUGIN_NAME 64

/*
    MPQDRAFTPLUGINMODULE
    
    Structure used by IMPQDraftPlugin::GetModules to notify MPQDraft of any
    files (called plugin modules) that are to be loaded.
*/
#include <pshpack1.h>
struct MPQDRAFTPLUGINMODULE
{
    DWORD dwComponentID;
    DWORD dwModuleID;
    BOOL bExecute;
    char szModuleFileName[MPQDRAFT_MAX_PATH];
};
#include <poppack.h>

/*
    IMPQDraftServer
    
    Serves as a portal back to MPQDraft, allowing the plugin to
    communicate with MPQDraft.
*/ 
struct IMPQDraftServer
{
    virtual BOOL WINAPI GetPluginModule(DWORD dwPluginID, DWORD dwModuleID, LPSTR lpszFileName) = 0;
};

/*
    IMPQDraftPlugin
    
    The primary gateway between the MPQDraft patching kernel and the plugin.
*/
struct IMPQDraftPlugin
{
    virtual BOOL WINAPI Identify(LPDWORD lpdwPluginID) = 0;
    virtual BOOL WINAPI GetPluginName(LPSTR lpszPluginName, DWORD nNameBufferLength) = 0;
    virtual BOOL WINAPI CanPatchExecutable(LPCSTR lpszEXEFileName) = 0;
    virtual BOOL WINAPI Configure(HWND hParentWnd) = 0;
    virtual BOOL WINAPI ReadyForPatch() = 0;
    virtual BOOL WINAPI GetModules(MPQDRAFTPLUGINMODULE *lpPluginModules, LPDWORD lpnNumModules) = 0;
    virtual BOOL WINAPI InitializePlugin(IMPQDraftServer *lpMPQDraftServer) = 0;
    virtual BOOL WINAPI TerminatePlugin() = 0;
};

/*
    GetMPQDraftPlugin
    
    Exported by name from the plugin DLL.
*/
BOOL WINAPI GetMPQDraftPlugin(IMPQDraftPlugin **lppMPQDraftPlugin);

#endif // #ifndef QDPLUGIN_H

