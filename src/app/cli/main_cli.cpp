/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// main_cli.cpp : CLI entry point logic

#include "main_cli.h"
#include <windows.h>
#include <stdio.h>
#include "../../common/QDebug.h"
#include "../../common/QResource.h"
#include "MPQDraftCLI.h"
#include "CommandParser.h"
#include "../resource_ids.h"
#include "version.h"

/////////////////////////////////////////////////////////////////////////////
// Helper function to extract patcher DLL

static BOOL GetPatcherDLLPath(char* szPatcherPath, DWORD dwBufferSize)
{
	// Extract the patcher DLL to a temporary location
	BOOL bSuccess = ExtractTempResource(NULL, MAKEINTRESOURCE(IDR_PATCHERDLL),
		"DLL", szPatcherPath);

	if (!bSuccess)
	{
		printf("Failed to extract patcher DLL\n");
		QDebugOut("Failed to extract patcher DLL");
	}

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CLI entry point - called from main entry point

int runCli(int argc, char** argv)
{
	// Parse command line
	CommandParser cmdParser;
	bool parseStatus = cmdParser.ParseCommandLine(argc, argv);

	// Handle help/version/list-games - these print messages and exit
	if (cmdParser.IsHelpRequested() || cmdParser.IsVersionRequested() ||
	    cmdParser.GetCommandType() == CommandType::ListGames)
	{
		printf("%s\n", cmdParser.GetMessage().c_str());
		return 0;
	}

	// Handle parse errors
	if (!parseStatus)
	{
		printf("%s\n", cmdParser.GetMessage().c_str());
		QDebugOut("Command line parsing error");
		return 1;
	}

	// Handle commands
	switch (cmdParser.GetCommandType())
	{
		case CommandType::Patch:
		{
			const PatchCommand& cmd = cmdParser.GetPatchCommand();

			// Get patcher DLL path
			char szPatcherPath[MAX_PATH + 1];
			if (!GetPatcherDLLPath(szPatcherPath, sizeof(szPatcherPath)))
			{
				printf("Failed to get patcher DLL path\n");
				QDebugOut("Failed to get patcher DLL path");
				return 1;
			}

			// Create CLI handler and execute
			CMPQDraftCLI cli;
			BOOL bSuccess = cli.ExecutePatch(cmd, szPatcherPath);
			return bSuccess ? 0 : 1;
		}

		case CommandType::SEMPQ:
		{
			const SEMPQCommand& cmd = cmdParser.GetSEMPQCommand();

			// Create CLI handler and execute
			CMPQDraftCLI cli;
			BOOL bSuccess = cli.ExecuteSEMPQ(cmd);
			return bSuccess ? 0 : 1;
		}

		case CommandType::None:
		case CommandType::ListGames:
		default:
			// Should not reach here
			printf("No command specified. Use --help for usage information.\n");
			return 1;
	}
}
