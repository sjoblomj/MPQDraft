/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// gamedata.cpp : Game definitions and supported programs
//

#include "gamedata.h"
#include "gui/resource.h"

// The lists of components for the supported programs
const PROGRAMFILEENTRY DiabloFiles[] =
{
	"Diablo", "Diablo.exe", "Diablo.exe", 1, IDI_DIABLO, TRUE, 0,
	NULL
};

const PROGRAMFILEENTRY DiabloHellfireFiles[] =
{
	"Diablo: Hellfire", "Hellfire.exe", "Hellfire.exe", 1, IDI_HELLFIRE, TRUE, 0,
	NULL
};

const PROGRAMFILEENTRY Diablo2Files[] =
{
	"Diablo II", "Diablo II.exe", "Game.exe", 0, IDI_DIABLO2, FALSE, 0,
	NULL
};

const PROGRAMFILEENTRY StarcraftFiles[] =
{
	"Starcraft", "Starcraft.exe", "Starcraft.exe", 0, IDI_STARCRAFT, TRUE, 0,
	"Campaign Editor", "StarEdit.exe", "StarEdit.exe", 0, IDI_STAREDIT, TRUE, 0,
	NULL
};

const PROGRAMFILEENTRY Warcraft2Files[] =
{
	"Warcraft II BNE", "Warcraft II BNE.exe", "Warcraft II BNE.exe", 0, IDI_WAR2BNE, TRUE, 0,
	//"Map Editor", "Warcraft II Map Editor.exe", "", IDI_WAR2EDIT, FALSE, 0
	NULL
};

const PROGRAMFILEENTRY Warcraft3Files[] =
{
	"Warcraft III", "Warcraft III.exe", "War3.exe", 0, IDI_WARCRAFT3, TRUE, 0,
	"The Frozen Throne", "Frozen Throne.exe", "War3.exe", 0, IDI_WAR3X, TRUE, 0,
	"World Editor", "World Editor.exe", "WorldEdit.exe", 0, IDI_WAR3EDIT, TRUE, 0,
	NULL
};

const PROGRAMFILEENTRY LordsOfMagicFiles[] =
{
	"Lords of Magic SE", "LOMSE.exe", "LOMSE.exe", 1, IDI_LOMSE, TRUE, 0,
	NULL
};

// The list of supported programs themselves
const PROGRAMENTRY SupportApps[] =
{
	"Diablo", "SOFTWARE\\Blizzard Entertainment\\Archives", "DiabloInstall", IDI_DIABLO, DiabloFiles,
	"Diablo: Hellfire", "SOFTWARE\\Sierra OnLine\\Setup\\HELLFIRE", "Directory", IDI_HELLFIRE, DiabloHellfireFiles,
	"Diablo II", "SOFTWARE\\Blizzard Entertainment\\Diablo II", "InstallPath", IDI_DIABLO2, Diablo2Files,
	"Starcraft", "SOFTWARE\\Blizzard Entertainment\\Starcraft", "InstallPath", IDI_STARCRAFT, StarcraftFiles,
	"Warcraft II BNE", "SOFTWARE\\Blizzard Entertainment\\Warcraft II BNE", "InstallPath", IDI_WAR2BNE, Warcraft2Files,
	"Warcraft III", "SOFTWARE\\Blizzard Entertainment\\Warcraft III", "InstallPath", IDI_WAR3HEADER, Warcraft3Files,
	//"Warcraft III Exp", "SOFTWARE\\Blizzard Entertainment\\Warcraft III", "InstallPathX", IDI_WAR3X, NULL,
	"Lords of Magic SE", "SOFTWARE\\Sierra OnLine\\Setup\\LOMSE", "Directory", IDI_LOMSE, LordsOfMagicFiles,
	NULL
};
