/*
    SEMPQ Creator - Mock implementation for Qt GUI
    
    This is a mock implementation that simulates the SEMPQ creation process
    for the Qt GUI. It mimics the three-step process from the original MFC code:
    1. Writing Executable Code (0% - 5%)
    2. Writing Plugins (5% - 20%)
    3. Writing MPQ Data (20% - 100%)
*/

#ifndef SEMPQ_CREATOR_H
#define SEMPQ_CREATOR_H

#include <QString>
#include <QStringList>
#include <functional>
#include <cstdint>
#include "mpqdraftplugin.h"  // For MPQDRAFTPLUGINMODULE

// Progress callback function type
// Parameters: progress (0-100), status text
using ProgressCallback = std::function<void(int, const QString&)>;

// Cancellation check function type
// Returns: true if operation should be cancelled
using CancellationCheck = std::function<bool()>;

// SEMPQ creation parameters
struct SEMPQCreationParams
{
    // Output file
    QString outputPath;
    
    // SEMPQ settings
    QString sempqName;
    QString mpqPath;
    QString iconPath;
    
    // Target settings
    bool useRegistry;
    QString registryKey;
    QString registryValue;
    bool valueIsFullPath;  // If true, registry value contains full path to .exe (not just directory)
    QString targetPath;
    QString targetFileName;
    QString spawnFileName;
    int shuntCount;
    
    // Flags and parameters
    uint32_t flags;
    QString parameters;
    
    // Plugins (with full metadata including component/module IDs)
    std::vector<MPQDRAFTPLUGINMODULE> pluginModules;
};

// SEMPQ Creator class
class SEMPQCreator
{
public:
    SEMPQCreator();
    ~SEMPQCreator();
    
    // Create a SEMPQ file with the given parameters
    // Returns true on success, false on failure
    // Calls progressCallback periodically with progress updates
    // Calls cancellationCheck periodically to check if operation should be cancelled
    bool createSEMPQ(
        const SEMPQCreationParams& params,
        ProgressCallback progressCallback,
        CancellationCheck cancellationCheck,
        QString& errorMessage
    );
    
private:
    // Step 1: Write executable code (0% - 5%)
    bool writeStubToSEMPQ(
        const SEMPQCreationParams& params,
        ProgressCallback progressCallback,
        CancellationCheck cancellationCheck,
        QString& errorMessage
    );
    
    // Step 2: Write plugins (5% - 20%)
    bool writePluginsToSEMPQ(
        const SEMPQCreationParams& params,
        ProgressCallback progressCallback,
        CancellationCheck cancellationCheck,
        QString& errorMessage
    );
    
    // Step 3: Write MPQ data (20% - 100%)
    bool writeMPQToSEMPQ(
        const SEMPQCreationParams& params,
        ProgressCallback progressCallback,
        CancellationCheck cancellationCheck,
        QString& errorMessage
    );
    
    // Helper: Simulate work by sleeping for a random duration
    void simulateWork(int minMs, int maxMs);
};

#endif // SEMPQ_CREATOR_H

