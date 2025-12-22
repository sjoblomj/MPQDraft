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
#include <shellapi.h>
#include <vector>
#include <string>
#include "../common/QResource.h"
#include "cli/main_cli.h"
#include "gui/main_gui.h"

/////////////////////////////////////////////////////////////////////////////
// Convert Windows command line to argc/argv format
// Uses CommandLineToArgvW for proper handling of quotes and escapes

static void getArgcArgv(int& argc, std::vector<std::string>& argStorage, std::vector<char*>& argv)
{
	// Get the full command line (including program name)
	LPWSTR cmdLine = GetCommandLineW();

	// Parse into wide string array
	int wargc = 0;
	LPWSTR* wargv = CommandLineToArgvW(cmdLine, &wargc);

	if (wargv == nullptr)
	{
		// Fallback: just program name
		argc = 1;
		argStorage.push_back("MPQDraft");
		argv.push_back(&argStorage[0][0]);
		argv.push_back(nullptr);
		return;
	}

	// Convert wide strings to UTF-8
	argc = wargc;
	argStorage.reserve(wargc);
	argv.reserve(wargc + 1);

	for (int i = 0; i < wargc; i++)
	{
		// Get required buffer size
		int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
		if (size > 0)
		{
			std::string arg(size - 1, '\0');  // size includes null terminator
			WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &arg[0], size, nullptr, nullptr);
			argStorage.push_back(std::move(arg));
		}
		else
		{
			argStorage.push_back("");
		}
	}

	// Build argv array pointing to the strings
	for (auto& arg : argStorage)
	{
		argv.push_back(&arg[0]);
	}
	argv.push_back(nullptr);

	LocalFree(wargv);
}

/////////////////////////////////////////////////////////////////////////////
// Check if command line has CLI arguments
// Returns true if we should run in CLI mode

static bool hasCLIArguments(int argc)
{
	// If we have more than just the program name, we have arguments
	return argc > 1;
}

/////////////////////////////////////////////////////////////////////////////
// Main entry point

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

	// Initialize QResource system (needed for both CLI and GUI)
	QResourceInitialize();

	// Convert command line to argc/argv
	int argc;
	std::vector<std::string> argStorage;
	std::vector<char*> argv;
	getArgcArgv(argc, argStorage, argv);

	int result;

	if (hasCLIArguments(argc))
	{
		// CLI mode - run command line interface
		result = runCli(argc, argv.data());
	}
	else
	{
		// GUI mode - run Qt GUI
		result = runQtGui(argc, argv.data());
	}

	// Clean up
	DeleteTemporaryFiles();
	QResourceDestroy();

	return result;
}
