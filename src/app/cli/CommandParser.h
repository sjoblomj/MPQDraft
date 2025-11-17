/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#pragma once
#include <windows.h>
#include <vector>
#include <string>

class CommandParser
{
public:
	// Parse command line arguments
	// Returns true on success, false on error
	bool ParseCommandLine(const char* lpCmdLine);

	// Get parsed results
	const std::string& GetTarget() const { return m_target; }
	const std::vector<std::string>& GetMPQs() const { return m_mpqs; }
	const std::vector<std::string>& GetPlugins() const { return m_plugins; }

	// Check if target was specified
	bool HasTarget() const { return !m_target.empty(); }

	// Get error message if parsing failed
	const std::string& GetErrorMessage() const { return m_errorMessage; }

private:
	std::string m_target;
	std::vector<std::string> m_mpqs;
	std::vector<std::string> m_plugins;
	std::string m_errorMessage;
};
