/*
    SEMPQ Creator - Mock implementation for Qt GUI

    This is a mock implementation that simulates the SEMPQ creation process
    for the Qt GUI. It mimics the three-step process from the original MFC code:
    1. Writing Executable Code (0% - 5%)
    2. Writing Plugins (5% - 20%)
    3. Writing MPQ Data (20% - 100%)
*/

#ifndef SEMPQ_CREATOR_MOCK_H
#define SEMPQ_CREATOR_MOCK_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "../../../core/MPQDraftPlugin.h"  // For MPQDRAFTPLUGINMODULE

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

// SEMPQ Creator class (Mock implementation)
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

    // Create a SEMPQ file with the given parameters
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

    // Helper: Simulate work by sleeping for a random duration
    void simulateWork(int minMs, int maxMs);
};

#endif // SEMPQ_CREATOR_MOCK_H
