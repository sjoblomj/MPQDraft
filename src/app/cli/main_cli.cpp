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

int runCli(const char* lpCmdLine)
{
	// Parse command line
	CommandParser cmdParser;
	bool parseStatus = cmdParser.ParseCommandLine(lpCmdLine);

	if (cmdParser.IsVersionRequested())
	{
		PrintVersion();
		return 0;
	}

	if (!parseStatus || !cmdParser.HasTarget() || cmdParser.IsHelpRequested())
	{
		if (!parseStatus)
		{
			// Parsing failed - show error and exit
			const std::string& errorMsg = cmdParser.GetErrorMessage();
			printf("Command line parsing error: %s\n\n", errorMsg.c_str());
			QDebugOut("Command line parsing error: %s", errorMsg.c_str());
		}
		if (!cmdParser.HasTarget() && !cmdParser.IsHelpRequested())
		{
			printf("No target executable specified\n\n");
		}
		PrintHelp();

		return cmdParser.IsHelpRequested() ? 0 : 1;
	}

	// Get command line parameters
	const std::string& target = cmdParser.GetTarget();
	const std::vector<std::string>& mpqs = cmdParser.GetMPQs();
	const std::vector<std::string>& plugins = cmdParser.GetPlugins();

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
	BOOL bSuccess = cli.Execute(target.c_str(), mpqs, plugins, szPatcherPath);

	return bSuccess ? 0 : 1;
}
