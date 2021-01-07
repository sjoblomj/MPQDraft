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