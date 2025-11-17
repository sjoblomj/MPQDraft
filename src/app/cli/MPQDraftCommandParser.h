/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#pragma once
#include <windows.h>
#include <vector>
#include <string>

class MPQDraftCommandParser
{
public:
	// Parse command line arguments
	void ParseCommandLine(int argc, char* argv[]);
	void ParseCommandLine(const char* lpCmdLine);

	// Get parsed results
	const std::vector<std::string>& GetParams() const { return m_params; }
	const std::vector<std::string>& GetSwitches() const { return m_switches; }

private:
	std::vector<std::string> m_params;
	std::vector<std::string> m_switches;
	void ParseParam(const char* param, BOOL isFlag);
};
