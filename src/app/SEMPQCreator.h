/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// SEMPQCreator.h - SEMPQ creation logic
//
// This file contains the core logic for creating Self-Executing MPQ (SEMPQ) files.

#ifndef SEMPQCREATOR_H
#define SEMPQCREATOR_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

// Forward declaration (to avoid including Windows headers)
struct MPQDRAFTPLUGINMODULE;

// Progress callback function type
// Parameters: progress (0-100), status text
using ProgressCallback = std::function<void(int, const std::string&)>;

// Cancellation check function type
// Returns: true if operation should be cancelled
using CancellationCheck = std::function<bool()>;

// SEMPQ creation parameters
struct SEMPQCreationParams
{
	// Output file
	std::string outputPath;

	// SEMPQ settings
	std::string sempqName;
	std::string mpqPath;
	std::string iconPath;

	// Target settings
	bool useRegistry;
	std::string registryKey;
	std::string registryValue;
	bool valueIsFullPath;  // If true, registry value contains full path to .exe (not just directory)
	std::string targetPath;
	std::string targetFileName;
	std::string spawnFileName;
	int shuntCount;

	// Flags and parameters
	uint32_t flags;
	std::string parameters;

	// Plugins (with full metadata including component/module IDs)
	std::vector<MPQDRAFTPLUGINMODULE> pluginModules;
};

/////////////////////////////////////////////////////////////////////////////
// SEMPQCreator - SEMPQ creation class
/////////////////////////////////////////////////////////////////////////////

class SEMPQCreator
{
public:
	// Progress range constants (in %)
	static constexpr int WRITE_STUB_INITIAL_PROGRESS = 0;
	static constexpr int WRITE_PLUGINS_INITIAL_PROGRESS = 5;
	static constexpr int WRITE_PLUGINS_PROGRESS_SIZE = 15;
	static constexpr int WRITE_MPQ_INITIAL_PROGRESS = 20;
	static constexpr int WRITE_MPQ_PROGRESS_SIZE = 80;
	static constexpr int WRITE_FINISHED = 100;

	// Main entry point: Create a complete SEMPQ file
	// Returns true on success, false on failure
	// Calls progressCallback periodically with progress updates
	// Calls cancellationCheck periodically to check if operation should be cancelled
	bool createSEMPQ(
		const SEMPQCreationParams& params,
		ProgressCallback progressCallback,
		CancellationCheck cancellationCheck,
		std::string& errorMessage
	);

private:
	// Step 1: Write executable code (0% - 5%)
	bool writeStubToSEMPQ(
		const SEMPQCreationParams& params,
		ProgressCallback progressCallback,
		CancellationCheck cancellationCheck,
		std::string& errorMessage
	);

	// Step 2: Write plugins (5% - 20%)
	bool writePluginsToSEMPQ(
		const SEMPQCreationParams& params,
		ProgressCallback progressCallback,
		CancellationCheck cancellationCheck,
		std::string& errorMessage
	);

	// Step 3: Write MPQ data (20% - 100%)
	bool writeMPQToSEMPQ(
		const SEMPQCreationParams& params,
		ProgressCallback progressCallback,
		CancellationCheck cancellationCheck,
		std::string& errorMessage
	);
};

#endif // SEMPQCREATOR_H
