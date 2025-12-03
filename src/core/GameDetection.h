/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Functions for locating installed games
#if !defined(GAMEDETECTION_H)
#define GAMEDETECTION_H

#include <string>

// Locates an installed game component file path through the game's registry entry
std::string locateComponent(const std::string& registryKey, const std::string& registryValue, const std::string& fileName);

#endif
