/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// sempqparamsbuilder.cpp - Implementation of SEMPQ parameter building logic

#include "sempqparamsbuilder.h"
#include "gamedata.h"
#include "../common/sempq_creator.h"
#include "../common/common.h"

bool SEMPQParamsBuilder::buildParams(
    const SEMPQBasicSettings& basicSettings,
    const SEMPQTargetSettings& targetSettings,
    const std::vector<MPQDRAFTPLUGINMODULE>& pluginModules,
    SEMPQCreationParams& outParams,
    std::string& errorMessage)
{
    // Copy basic settings
    outParams.sempqName = basicSettings.sempqName;
    outParams.mpqPath = basicSettings.mpqPath;
    outParams.iconPath = basicSettings.iconPath;
    outParams.outputPath = basicSettings.outputPath;
    outParams.parameters = basicSettings.parameters;

    // Copy plugin modules
    outParams.pluginModules = pluginModules;

    // Build target-specific parameters based on mode
    switch (targetSettings.mode) {
        case SEMPQTargetMode::SUPPORTED_GAME:
            return buildSupportedGameParams(targetSettings, outParams, errorMessage);

        case SEMPQTargetMode::CUSTOM_REGISTRY:
            return buildCustomRegistryParams(targetSettings, outParams, errorMessage);

        case SEMPQTargetMode::CUSTOM_PATH:
            return buildCustomPathParams(targetSettings, outParams, errorMessage);

        default:
            errorMessage = "Internal error: unknown target mode";
            return false;
    }
}

bool SEMPQParamsBuilder::buildSupportedGameParams(
    const SEMPQTargetSettings& targetSettings,
    SEMPQCreationParams& params,
    std::string& errorMessage)
{
    // Validate that we have a selected component
    if (!targetSettings.selectedComponent) {
        errorMessage = "No game component selected";
        return false;
    }

    const GameComponent* component = targetSettings.selectedComponent;

    // Set registry-based mode
    params.useRegistry = true;

    // Find the parent game to get registry information
    // We need to search through all supported games to find which one contains this component
    std::vector<SupportedGame> games = getSupportedGames();
    bool foundGame = false;

    for (const SupportedGame& game : games) {
        for (const GameComponent& comp : game.components) {
            // Compare by component name and file name to find the match
            if (comp.componentName == component->componentName &&
                comp.fileName == component->fileName) {
                // Found the parent game - extract registry info
                params.registryKey = game.registryKey;
                params.registryValue = game.registryValue;
                foundGame = true;
                break;
            }
        }
        if (foundGame)
            break;
    }

    if (!foundGame) {
        errorMessage = "Internal error: could not find parent game for selected component";
        return false;
    }

    // Set component-specific parameters
    params.targetFileName = component->targetFileName;
    params.spawnFileName = component->fileName;
    params.shuntCount = component->shuntCount;
    params.valueIsFullPath = false;  // Supported games always use directory + filename

    // Set flags
    params.flags = 0;
    if (targetSettings.extendedRedir) {
        params.flags |= MPQD_EXTENDED_REDIR;
    }

    return true;
}

bool SEMPQParamsBuilder::buildCustomRegistryParams(
    const SEMPQTargetSettings& targetSettings,
    SEMPQCreationParams& params,
    std::string& errorMessage)
{
    // Validate required fields
    if (targetSettings.customRegistryKey.empty()) {
        errorMessage = "Custom registry key is empty";
        return false;
    }

    if (targetSettings.customRegistryValue.empty()) {
        errorMessage = "Custom registry value is empty";
        return false;
    }

    if (targetSettings.customRegistryExe.empty()) {
        errorMessage = "Custom registry exe file is empty";
        return false;
    }

    // Set registry-based mode
    params.useRegistry = true;

    // Copy custom registry settings
    params.registryKey = targetSettings.customRegistryKey;
    params.registryValue = targetSettings.customRegistryValue;
    params.valueIsFullPath = targetSettings.customRegistryIsFullPath;
    params.spawnFileName = targetSettings.customRegistryExe;
    params.targetFileName = targetSettings.customRegistryTargetFile;
    params.shuntCount = targetSettings.customRegistryShuntCount;
    params.flags = targetSettings.customRegistryFlags;

    return true;
}

bool SEMPQParamsBuilder::buildCustomPathParams(
    const SEMPQTargetSettings& targetSettings,
    SEMPQCreationParams& params,
    std::string& errorMessage)
{
    // Validate required fields
    if (targetSettings.customTargetPath.empty()) {
        errorMessage = "Custom target path is empty";
        return false;
    }

    // Set hardcoded path mode
    params.useRegistry = false;

    // Copy custom path settings
    params.targetPath = targetSettings.customTargetPath;
    params.shuntCount = targetSettings.customTargetShuntCount;

    // Set flags
    params.flags = 0;
    if (targetSettings.extendedRedir) {
        params.flags |= MPQD_EXTENDED_REDIR;
    }
    if (targetSettings.customTargetNoSpawning) {
        params.flags |= MPQD_NO_SPAWNING;
    }

    return true;
}
