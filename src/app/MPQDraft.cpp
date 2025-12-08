/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// MPQDraft.cpp : Main entry point for MPQDraft
//
// This file contains the main entry point that dispatches to either:
// - CLI mode (when command line arguments are provided)
// - GUI mode (when no arguments are provided)

#include <windows.h>
#include "../common/QResource.h"
#include "cli/main_cli.h"
#include "gui/main_gui.h"

/////////////////////////////////////////////////////////////////////////////
// Check if command line has CLI arguments
// Returns true if we should run in CLI mode

static bool hasCLIArguments(const char* lpCmdLine)
{
	if (!lpCmdLine)
		return false;

	// Skip leading whitespace
	while (*lpCmdLine == ' ' || *lpCmdLine == '\t')
		lpCmdLine++;

	// If there's anything left, we have arguments
	return *lpCmdLine != '\0';
}

/////////////////////////////////////////////////////////////////////////////
// Main entry point

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize QResource system (needed for both CLI and GUI)
	QResourceInitialize();

	int result;

	if (hasCLIArguments(lpCmdLine))
	{
		// CLI mode - run command line interface
		result = runCli(lpCmdLine);
	}
	else
	{
		// GUI mode - run Qt GUI
		// Convert to argc/argv format for Qt
		int argc = 1;
		char* argv[] = { (char*)"MPQDraft", nullptr };
		result = runQtGui(argc, argv);
	}

	// Clean up
	DeleteTemporaryFiles();
	QResourceDestroy();

	return result;
}
