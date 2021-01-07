#pragma once
#include <afxwin.h>
class MPQDraftCommandParser : public CCommandLineInfo
{
public:
	MPQDraftCommandParser(void);
	virtual ~MPQDraftCommandParser(void);
	void GetParams(CStringArray& params);
	void GetSwitches(CStringArray& switches);
private:
	CStringArray m_params;
	CStringArray m_switches;
	void ParseParam(const TCHAR* param, BOOL isFlag, BOOL isLast);
};