/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "MPQDraftCommandParser.h"
#include <cstring>

void MPQDraftCommandParser::ParseParam(const char* param, BOOL isFlag)
{
	if (isFlag) {
		m_switches.push_back(param);
	}
	else {
		m_params.push_back(param);
	}
}

void MPQDraftCommandParser::ParseCommandLine(int argc, char* argv[])
{
	m_params.clear();
	m_switches.clear();

	// Skip argv[0] which is the program name
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		BOOL isFlag = (arg[0] == '-' || arg[0] == '/');

		if (isFlag) {
			// Skip the leading - or /
			ParseParam(arg + 1, TRUE);
		}
		else {
			ParseParam(arg, FALSE);
		}
	}
}

void MPQDraftCommandParser::ParseCommandLine(const char* lpCmdLine)
{
	m_params.clear();
	m_switches.clear();

	if (!lpCmdLine || lpCmdLine[0] == '\0')
		return;

	// Simple command line parser
	// This handles quoted strings and splits on spaces
	std::string cmdLine = lpCmdLine;
	size_t pos = 0;

	while (pos < cmdLine.length())
	{
		// Skip whitespace
		while (pos < cmdLine.length() && (cmdLine[pos] == ' ' || cmdLine[pos] == '\t'))
			pos++;

		if (pos >= cmdLine.length())
			break;

		// Check if this is a flag
		BOOL isFlag = (cmdLine[pos] == '-' || cmdLine[pos] == '/');
		if (isFlag)
			pos++; // Skip the flag character

		// Parse the argument
		std::string arg;
		BOOL inQuotes = FALSE;

		while (pos < cmdLine.length())
		{
			char c = cmdLine[pos];

			if (c == '"') {
				inQuotes = !inQuotes;
				pos++;
			}
			else if (!inQuotes && (c == ' ' || c == '\t')) {
				break;
			}
			else {
				arg += c;
				pos++;
			}
		}

		if (!arg.empty()) {
			ParseParam(arg.c_str(), isFlag);
		}
	}
}
