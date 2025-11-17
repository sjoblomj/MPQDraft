/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "stdafx_cli.h"
#include "CommandParser.h"
#include <cstring>

void SkipWhitespace(const std::string& str, size_t& pos)
{
	while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\r' || str[pos] == '\n'))
		pos++;
}

std::string ParseArgument(const std::string& cmdLine, size_t& pos)
{
	SkipWhitespace(cmdLine, pos);

	if (pos >= cmdLine.length())
		return "";

	std::string arg;
	BOOL inQuotes = FALSE;
	size_t startPos = pos;

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

	// Trim trailing whitespace from the argument
	size_t endPos = arg.length();
	while (endPos > 0 && (arg[endPos - 1] == ' ' || arg[endPos - 1] == '\t' || arg[endPos - 1] == '\r' || arg[endPos - 1] == '\n'))
		endPos--;

	return arg.substr(0, endPos);
}

bool CommandParser::ParseCommandLine(const char* lpCmdLine)
{
	m_target.clear();
	m_mpqs.clear();
	m_plugins.clear();
	m_errorMessage.clear();

	if (!lpCmdLine || lpCmdLine[0] == '\0')
		return true;

	std::string cmdLine = lpCmdLine;
	size_t pos = 0;

	// Skip the program name (first argument)
	ParseArgument(cmdLine, pos);

	while (pos < cmdLine.length())
	{
		SkipWhitespace(cmdLine, pos);

		if (pos >= cmdLine.length())
			break;

		// Check if this is a flag
		if (cmdLine[pos] != '-')
		{
			// Unexpected argument without a flag - ParseArgument will skip whitespace
			std::string unexpectedArg = ParseArgument(cmdLine, pos);
			m_errorMessage = "Unexpected argument: '" + unexpectedArg + "'. All arguments must be preceded by a flag (--target, --mpq, or --plugin).";
			return false;
		}

		pos++; // Skip the first '-'

		// Skip second dash if present (for --flag syntax)
		if (pos < cmdLine.length() && cmdLine[pos] == '-')
			pos++;

		// Parse the flag name (everything until whitespace or quote)
		std::string flag;
		while (pos < cmdLine.length() && cmdLine[pos] != ' ' && cmdLine[pos] != '\t' && cmdLine[pos] != '"')
		{
			flag += cmdLine[pos];
			pos++;
		}

		if (flag.empty())
		{
			m_errorMessage = "Empty flag encountered. Expected --target, --mpq, or --plugin.";
			return false;
		}

		// Determine which flag this is
		bool isTarget = (flag == "target" || flag == "t");
		bool isMPQ = (flag == "mpq" || flag == "m");
		bool isPlugin = (flag == "plugin" || flag == "p");

		if (!isTarget && !isMPQ && !isPlugin)
		{
			m_errorMessage = "Unknown flag: '--" + flag + "'. Valid flags are: --target/-t, --mpq/-m, --plugin/-p.";
			return false;
		}

		// Parse the value for this flag
		std::string value = ParseArgument(cmdLine, pos);

		if (value.empty())
		{
			m_errorMessage = "Flag '--" + flag + "' requires a value.";
			return false;
		}

		// Store the value
		if (isTarget)
		{
			m_target = value;
		}
		else if (isMPQ)
		{
			m_mpqs.push_back(value);
		}
		else if (isPlugin)
		{
			m_plugins.push_back(value);
		}
	}

	return true;
}
