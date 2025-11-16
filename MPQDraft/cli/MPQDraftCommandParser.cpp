/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "MPQDraftCommandParser.h"

void MPQDraftCommandParser::ParseParam(const TCHAR* param, BOOL isFlag, BOOL isLast)
{
	CString param_or_switch(param);
	if (isFlag) {
		m_switches.Add(param_or_switch);
	}
	else {
		m_params.Add(param_or_switch);
	}
}

MPQDraftCommandParser::MPQDraftCommandParser(void)
{
}

MPQDraftCommandParser::~MPQDraftCommandParser(void)
{
}

void MPQDraftCommandParser::GetParams(CStringArray& params)
{
	int size = m_params.GetCount();
	for (int i = 0; i < size; i++)
		params.Add(m_params.GetAt(i));
}
void MPQDraftCommandParser::GetSwitches(CStringArray& switches)
{
	int size = m_switches.GetCount();
	for (int i = 0; i < size; i++)
		switches.Add(m_switches.GetAt(i));
}
