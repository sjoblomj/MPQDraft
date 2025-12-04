/*
	Adapter layer to bridge the new gamedata structures (SupportedGame, GameComponent)
	to the legacy MFC GUI code that uses PROGRAMENTRY and PROGRAMFILEENTRY.
	This is temporary code that will be removed when the MFC GUI is replaced with Qt.
*/

#include "GameDataAdapter.h"
#include "resource.h"
#include <vector>
#include <string>

// Storage for the legacy data
static std::vector<SupportedGame> g_games;
static std::vector<PROGRAMENTRY> g_programEntries;
static std::vector<std::vector<PROGRAMFILEENTRY>> g_fileEntries;
static std::vector<std::string> g_strings;  // Keep strings alive

const PROGRAMENTRY *SupportApps = nullptr;

// Helper to store a string and return a pointer to it
static LPCSTR storeString(const std::string& str) {
	g_strings.push_back(str);
	return g_strings.back().c_str();
}

// Convert icon path string to resource ID
static DWORD iconPathToResourceId(const std::string& iconPath) {
	if (iconPath == ":/icons/blizzard/Diablo.ico") return IDI_DIABLO;
	if (iconPath == ":/icons/blizzard/hellfire.ico") return IDI_HELLFIRE;
	if (iconPath == ":/icons/blizzard/Diablo2.ico") return IDI_DIABLO2;
	if (iconPath == ":/icons/blizzard/Starcraft.ico") return IDI_STARCRAFT;
	if (iconPath == ":/icons/blizzard/StarEdit.ico") return IDI_STAREDIT;
	if (iconPath == ":/icons/blizzard/War2BNE.ico") return IDI_WAR2BNE;
	if (iconPath == ":/icons/blizzard/War3Head.ico") return IDI_WAR3HEADER;
	if (iconPath == ":/icons/blizzard/War3.ico") return IDI_WARCRAFT3;
	if (iconPath == ":/icons/blizzard/War3x.ico") return IDI_WAR3X;
	if (iconPath == ":/icons/blizzard/War3Edit.ico") return IDI_WAR3EDIT;
	if (iconPath == ":/icons/sierra/lomse.ico") return IDI_LOMSE;
	return 0;
}

void InitLegacyGameData() {
	// Clean up any existing data
	CleanupLegacyGameData();

	// Get the new game data
	g_games = getSupportedGames();

	// Reserve space
	g_programEntries.reserve(g_games.size() + 1);  // +1 for null terminator
	g_fileEntries.reserve(g_games.size());

	// Convert each game
	for (size_t i = 0; i < g_games.size(); i++) {
		const SupportedGame& game = g_games[i];

		// Create file entries for this game's components
		std::vector<PROGRAMFILEENTRY> files;
		files.reserve(game.components.size() + 1);  // +1 for null terminator

		for (const GameComponent& comp : game.components) {
			PROGRAMFILEENTRY file = {};
			file.szComponentName = storeString(comp.componentName);
			file.szFileName = storeString(comp.fileName);
			file.szTargetFileName = storeString(comp.targetFileName);
			file.nShuntCount = comp.shuntCount;
			file.iIcon = iconPathToResourceId(comp.iconPath);
			file.bExtendRedir = comp.extendedRedir ? TRUE : FALSE;
			file.dwFlags = comp.flags;
			files.push_back(file);
		}

		// Add null terminator entry
		PROGRAMFILEENTRY nullFile = {};
		nullFile.szComponentName = nullptr;
		files.push_back(nullFile);

		g_fileEntries.push_back(std::move(files));

		// Create program entry
		PROGRAMENTRY prog = {};
		prog.szProgramName = storeString(game.gameName);
		prog.szRegistryKey = storeString(game.registryKey);
		prog.szRegistryValue = storeString(game.registryValue);
		prog.iIcon = iconPathToResourceId(game.iconPath);
		prog.files = g_fileEntries.back().data();
		g_programEntries.push_back(prog);
	}

	// Add null terminator entry
	PROGRAMENTRY nullProg = {};
	nullProg.szProgramName = nullptr;
	g_programEntries.push_back(nullProg);

	// Set the global pointer
	SupportApps = g_programEntries.data();
}

void CleanupLegacyGameData() {
	SupportApps = nullptr;
	g_programEntries.clear();
	g_fileEntries.clear();
	g_strings.clear();
	g_games.clear();
}
