/*
	Adapter layer to bridge the new gamedata structures (SupportedGame, GameComponent)
	to the legacy MFC GUI code that uses PROGRAMENTRY and PROGRAMFILEENTRY.
	This is temporary code that will be removed when the MFC GUI is replaced with Qt.
*/

#ifndef GAMEDATAADAPTER_H
#define GAMEDATAADAPTER_H

#include "../../core/GameData.h"
#include <windows.h>

// Legacy structure for a game component (executable)
struct PROGRAMFILEENTRY {
	LPCSTR szComponentName;
	LPCSTR szFileName;
	LPCSTR szTargetFileName;
	int nShuntCount;
	DWORD iIcon;
	BOOL bExtendRedir;
	DWORD dwFlags;
};

// Legacy structure for a supported game
struct PROGRAMENTRY {
	LPCSTR szProgramName;
	LPCSTR szRegistryKey;
	LPCSTR szRegistryValue;
	DWORD iIcon;
	const PROGRAMFILEENTRY *files;
};

// Global array of supported apps in legacy format
extern const PROGRAMENTRY *SupportApps;

// Initialize the legacy game data from the new format
// Must be called before using SupportApps
void InitLegacyGameData();

// Clean up the legacy game data
void CleanupLegacyGameData();

#endif // GAMEDATAADAPTER_H
