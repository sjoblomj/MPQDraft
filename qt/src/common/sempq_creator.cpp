/*
    SEMPQ Creator - Mock implementation for Qt GUI
*/

#include "sempq_creator.h"
#include <QThread>
#include <QRandomGenerator>
#include <QFileInfo>

// Progress range constants (matching original MFC code)
constexpr int WRITE_STUB_INITIAL_PROGRESS = 0;
constexpr int WRITE_PLUGINS_INITIAL_PROGRESS = 5;
constexpr int WRITE_PLUGINS_PROGRESS_SIZE = 15;
constexpr int WRITE_MPQ_INITIAL_PROGRESS = 20;
constexpr int WRITE_MPQ_PROGRESS_SIZE = 80;

SEMPQCreator::SEMPQCreator()
{
}

SEMPQCreator::~SEMPQCreator()
{
}

void SEMPQCreator::simulateWork(int minMs, int maxMs)
{
    int duration = QRandomGenerator::global()->bounded(minMs, maxMs + 1);
    QThread::msleep(duration);
}

bool SEMPQCreator::createSEMPQ(
    const SEMPQCreationParams& params,
    ProgressCallback progressCallback,
    CancellationCheck cancellationCheck,
    std::string& errorMessage)
{
    // Validate parameters
    if (params.outputPath.empty())
    {
        errorMessage = "Output path is empty";
        return false;
    }

    if (params.sempqName.empty())
    {
        errorMessage = "SEMPQ name is empty";
        return false;
    }

    if (params.mpqPath.empty())
    {
        errorMessage = "MPQ path is empty";
        return false;
    }

    // Check if MPQ file exists
    QFileInfo mpqInfo(QString::fromStdString(params.mpqPath));
    if (!mpqInfo.exists())
    {
        errorMessage = "The MPQ " + params.mpqPath + " does not exist.";
        return false;
    }

    // Step 1: Write stub to SEMPQ
    if (!writeStubToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
        return false;

    // Step 2: Write plugins to SEMPQ
    if (!writePluginsToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
        return false;

    // Step 3: Write MPQ to SEMPQ
    if (!writeMPQToSEMPQ(params, progressCallback, cancellationCheck, errorMessage))
        return false;

    // Success!
    progressCallback(100, "SEMPQ created successfully!");
    return true;
}

bool SEMPQCreator::writeStubToSEMPQ(
    const SEMPQCreationParams& params,
    ProgressCallback progressCallback,
    CancellationCheck cancellationCheck,
    std::string& errorMessage)
{
    progressCallback(WRITE_STUB_INITIAL_PROGRESS, "Writing Executable Code...\n");

    // Simulate extracting the stub executable from resources
    simulateWork(1000, 3000);

    if (cancellationCheck && cancellationCheck())
    {
        errorMessage = "Operation cancelled by user";
        return false;
    }

    // Simulate finding the stub data offset
    simulateWork(500, 1500);

    if (cancellationCheck && cancellationCheck())
    {
        errorMessage = "Operation cancelled by user";
        return false;
    }

    // Simulate writing the stub data
    simulateWork(1000, 2000);

    if (cancellationCheck && cancellationCheck())
    {
        errorMessage = "Operation cancelled by user";
        return false;
    }

    progressCallback(WRITE_PLUGINS_INITIAL_PROGRESS, "Writing Executable Code...\n");
    return true;
}

bool SEMPQCreator::writePluginsToSEMPQ(
    const SEMPQCreationParams& params,
    ProgressCallback progressCallback,
    CancellationCheck cancellationCheck,
    std::string& errorMessage)
{
    progressCallback(WRITE_PLUGINS_INITIAL_PROGRESS, "Writing Plugins...\n");

    int numPlugins = static_cast<int>(params.pluginModules.size());

    // If no plugins, just simulate a quick operation
    if (numPlugins == 0)
    {
        simulateWork(500, 1500);
        progressCallback(WRITE_MPQ_INITIAL_PROGRESS, "Writing Plugins...\n");
        return true;
    }

    // Simulate writing each plugin
    for (int i = 0; i < numPlugins; i++)
    {
        // Simulate adding plugin to EFS file
        simulateWork(1000, 3000);

        // Update progress
        int progress = (int)(((float)i * (float)WRITE_PLUGINS_PROGRESS_SIZE / (float)numPlugins)
                            + (float)WRITE_PLUGINS_INITIAL_PROGRESS);
        progressCallback(progress, "Writing Plugins...\n");

        // Check for cancellation
        if (cancellationCheck && cancellationCheck())
        {
            errorMessage = "Operation cancelled by user";
            return false;
        }
    }

    progressCallback(WRITE_MPQ_INITIAL_PROGRESS, "Writing Plugins...\n");
    return true;
}

bool SEMPQCreator::writeMPQToSEMPQ(
    const SEMPQCreationParams& params,
    ProgressCallback progressCallback,
    CancellationCheck cancellationCheck,
    std::string& errorMessage)
{
    progressCallback(WRITE_MPQ_INITIAL_PROGRESS, "Writing MPQ Data...\n");

    // Get MPQ file size to simulate realistic progress
    QFileInfo mpqInfo(QString::fromStdString(params.mpqPath));
    qint64 mpqSize = mpqInfo.size();

    // Simulate copying MPQ data in chunks (256KB chunks like the original)
    const qint64 chunkSize = 256 * 1024;
    qint64 transferred = 0;

    while (transferred < mpqSize)
    {
        // Simulate reading and writing a chunk
        qint64 currentChunk = qMin(chunkSize, mpqSize - transferred);

        // Simulate work proportional to chunk size (smaller chunks = less time)
        int workTime = (int)((currentChunk * 2000) / chunkSize);  // 500-2000ms per chunk
        simulateWork(workTime / 2, workTime);

        transferred += currentChunk;

        // Update progress
        int progress = (int)(((float)transferred * WRITE_MPQ_PROGRESS_SIZE / (float)mpqSize)
                            + WRITE_MPQ_INITIAL_PROGRESS);
        progressCallback(progress, "Writing MPQ Data...\n");

        // Check for cancellation
        if (cancellationCheck && cancellationCheck())
        {
            errorMessage = "Operation cancelled by user";
            return false;
        }
    }

    // Simulate finalizing the file
    simulateWork(500, 1000);

    progressCallback(100, "Writing MPQ Data...\n");
    return true;
}
