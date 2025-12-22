/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#pragma once
#include <vector>
#include <string>

// Command types
enum class CommandType {
	None,           // No command (help/version requested or error)
	Patch,          // Patch and launch a game
	SEMPQ,          // Create a Self-Executing MPQ
	ListGames       // List supported games
};

// SEMPQ target mode
enum class SEMPQTargetMode {
	SupportedGame,  // Use a supported game from the list
	CustomRegistry, // Custom registry key/value
	CustomTarget    // Direct path to executable
};

// Parsed command line data for patch command
struct PatchCommand {
	std::string target;                 // Target executable path
	std::string parameters;             // Command-line parameters for target
	std::vector<std::string> mpqs;      // MPQ files to load
	std::vector<std::string> plugins;   // Plugin files to load
	bool extendedRedir = true;          // MPQD_EXTENDED_REDIR flag
	bool noSpawning = false;            // MPQD_NO_SPAWNING flag
	int shuntCount = 0;                 // Shunt count
};

// Parsed command line data for SEMPQ command
struct SEMPQCommand {
	SEMPQTargetMode mode = SEMPQTargetMode::SupportedGame;

	// Output
	std::string outputPath;             // Output SEMPQ file path
	std::string sempqName;              // Display name for the SEMPQ

	// Supported game mode
	std::string gameName;               // Game alias (e.g., "Starcraft", "StarEdit")

	// Custom registry mode
	std::string registryKey;            // Registry key
	std::string registryValue;          // Registry value name
	std::string exeFileName;            // Executable filename (spawn file)
	std::string targetFileName;         // Target filename for patching
	bool fullPath = false;              // Registry value is full path (MPQD_FULL_PATH)

	// Custom target mode
	std::string targetPath;             // Direct path to executable

	// Common options
	std::string mpqPath;                // MPQ file to embed
	std::vector<std::string> plugins;   // Plugin files to embed
	std::string parameters;             // Command-line parameters
	bool extendedRedir = true;          // MPQD_EXTENDED_REDIR flag
	bool noSpawning = false;            // MPQD_NO_SPAWNING flag
	int shuntCount = 0;                 // Shunt count
	std::string iconPath;               // Custom icon path
};

class CommandParser
{
public:
	// Parse command line arguments
	// Returns true on success, false on error
	bool ParseCommandLine(int argc, char** argv);

	// Get the command type
	CommandType GetCommandType() const { return m_commandType; }

	// Get parsed command data
	const PatchCommand& GetPatchCommand() const { return m_patchCommand; }
	const SEMPQCommand& GetSEMPQCommand() const { return m_sempqCommand; }

	// Check status flags
	bool IsHelpRequested() const { return m_helpRequested; }
	bool IsVersionRequested() const { return m_versionRequested; }

	// Get error/help message
	const std::string& GetMessage() const { return m_message; }

private:
	CommandType m_commandType = CommandType::None;
	PatchCommand m_patchCommand;
	SEMPQCommand m_sempqCommand;
	std::string m_message;
	bool m_helpRequested = false;
	bool m_versionRequested = false;
};
