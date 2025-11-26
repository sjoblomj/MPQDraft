/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// sempqparamsbuilder.h - Frontend-agnostic business logic for building SEMPQ creation parameters
//
// This class extracts the complex parameter building logic from the GUI layer,
// making it testable and reusable without any frontend framework dependencies.

#ifndef SEMPQPARAMSBUILDER_H
#define SEMPQPARAMSBUILDER_H

#include <string>
#include <vector>
#include <cstdint>
#include "../common/mpqdraftplugin.h"  // For MPQDRAFTPLUGINMODULE

// Forward declarations
struct GameComponent;
struct SEMPQCreationParams;

/////////////////////////////////////////////////////////////////////////////
// Input Data Structures (Frontend-agnostic representations of wizard page data)
/////////////////////////////////////////////////////////////////////////////

// Basic SEMPQ settings from the settings page
struct SEMPQBasicSettings {
    std::string sempqName;
    std::string mpqPath;
    std::string iconPath;
    std::string outputPath;
    std::string parameters;
};

// Target mode enumeration
enum class SEMPQTargetMode {
    SUPPORTED_GAME,    // Mode 1: Registry-based with supported game
    CUSTOM_REGISTRY,   // Mode 2: Registry-based with custom registry settings
    CUSTOM_PATH        // Mode 3: Hardcoded path (no registry)
};

// Target settings from the target page
struct SEMPQTargetSettings {
    SEMPQTargetMode mode;
    bool extendedRedir;

    // Mode 1: Supported Game
    const GameComponent* selectedComponent;  // Non-owning pointer

    // Mode 2: Custom Registry
    std::string customRegistryKey;
    std::string customRegistryValue;
    std::string customRegistryExe;
    std::string customRegistryTargetFile;
    int customRegistryShuntCount;
    bool customRegistryIsFullPath;
    uint32_t customRegistryFlags;

    // Mode 3: Custom Path
    std::string customTargetPath;
    int customTargetShuntCount;
    bool customTargetNoSpawning;

    SEMPQTargetSettings()
        : mode(SEMPQTargetMode::SUPPORTED_GAME)
        , extendedRedir(false)
        , selectedComponent(nullptr)
        , customRegistryShuntCount(0)
        , customRegistryIsFullPath(false)
        , customRegistryFlags(0)
        , customTargetShuntCount(0)
        , customTargetNoSpawning(false)
    {
    }
};

/////////////////////////////////////////////////////////////////////////////
// SEMPQParamsBuilder - Business logic for building SEMPQ creation parameters
/////////////////////////////////////////////////////////////////////////////

class SEMPQParamsBuilder {
public:
    // Build complete SEMPQCreationParams from structured input data
    // Returns true on success, false on failure (with errorMessage set)
    static bool buildParams(
        const SEMPQBasicSettings& basicSettings,
        const SEMPQTargetSettings& targetSettings,
        const std::vector<MPQDRAFTPLUGINMODULE>& pluginModules,
        SEMPQCreationParams& outParams,
        std::string& errorMessage
    );

private:
    // Build parameters for Mode 1: Supported Game (registry-based)
    static bool buildSupportedGameParams(
        const SEMPQTargetSettings& targetSettings,
        SEMPQCreationParams& params,
        std::string& errorMessage
    );

    // Build parameters for Mode 2: Custom Registry
    static bool buildCustomRegistryParams(
        const SEMPQTargetSettings& targetSettings,
        SEMPQCreationParams& params,
        std::string& errorMessage
    );

    // Build parameters for Mode 3: Custom Path (hardcoded)
    static bool buildCustomPathParams(
        const SEMPQTargetSettings& targetSettings,
        SEMPQCreationParams& params,
        std::string& errorMessage
    );
};

#endif // SEMPQPARAMSBUILDER_H
