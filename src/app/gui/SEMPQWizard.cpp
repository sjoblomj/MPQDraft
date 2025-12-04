/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// SEMPQWizard.cpp : implementation file
//

#include "stdafx_gui.h"
#include "../MPQDraft.h"
#include "../SEMPQCreator.h"
#include "resource.h"
#include "ProgressWnd.h"
#include "SEMPQWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage1 property page

IMPLEMENT_DYNCREATE(CSEMPQWizardPage1, CPropertyPage)

CSEMPQWizardPage1::CSEMPQWizardPage1() : CPropertyPage(CSEMPQWizardPage1::IDD)
{
	//{{AFX_DATA_INIT(CSEMPQWizardPage1)
	//}}AFX_DATA_INIT
}

CSEMPQWizardPage1::~CSEMPQWizardPage1()
{
}

void CSEMPQWizardPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSEMPQWizardPage1)
	DDX_Control(pDX, IDC_SEMPQNAME, m_ctlCustomName);
	DDX_Control(pDX, IDC_MPQNAME, m_ctlMPQName);
	DDX_Control(pDX, IDC_EXENAME, m_ctlEXEName);
	DDX_Control(pDX, IDC_ICONNAME, m_ctlIconName);
	DDX_Control(pDX, IDC_ICONDISPLAY, m_ctlIconDisplay);
	//}}AFX_DATA_MAP
}

BOOL CSEMPQWizardPage1::OnSetActive() 
{
	// Set the wizard buttons to show
	CSEMPQWizard *pWizard = (CSEMPQWizard *)GetParent();
	pWizard->SetWizardButtons(PSWIZB_NEXT);
	pWizard->ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	return CPropertyPage::OnSetActive();
}

BOOL CSEMPQWizardPage1::OnKillActive() 
{
	CString strMessage;

	if (!CPropertyPage::OnKillActive())
		return FALSE;

	// Verify each item. If one of them is missing, set the focus to it and 
	// warn the user. Need to get the MPQ file name here, because we need to 
	// check if it exists.
	CString strMPQName;
	m_ctlMPQName.GetWindowText(strMPQName);

	if (!m_ctlCustomName.GetWindowTextLength())
	{
		// No mod name
		strMessage.LoadString(IDS_NOSEMPQNAME);
		MessageBox(strMessage, NULL, MB_OK | MB_ICONEXCLAMATION);

		m_ctlCustomName.SetFocus();
		return FALSE;
	}
	else if (!strMPQName.GetLength() || !PathFileExists(strMPQName))
	{
		// No MPQ
		strMessage.LoadString(IDS_NOSEMPQMPQ);
		MessageBox(strMessage, NULL, MB_OK | MB_ICONEXCLAMATION);

		GetDlgItem(IDC_BROWSEMPQ)->SetFocus();
		return FALSE;
	}
	else if (!m_ctlEXEName.GetWindowTextLength())
	{
		// No output file name
		strMessage.LoadString(IDS_NOSEMPQFILENAME);
		MessageBox(strMessage, NULL, MB_OK | MB_ICONEXCLAMATION);

		GetDlgItem(IDC_BROWSEEXE)->SetFocus();
		return FALSE;
	}
	/*else if (!strIconName.GetLength())
	{
		// No icon. Check if they want to use the default icon.
		strMessage.LoadString(IDS_USEDEFSEMPQICON);
		if (MessageBox(strMessage, NULL, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			GetDlgItem(IDC_BROWSEICON)->SetFocus;
			return FALSE;
		}
	}*/

	return TRUE;	// Looks good
}

BEGIN_MESSAGE_MAP(CSEMPQWizardPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CSEMPQWizardPage1)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSEMPQ, OnBrowseMPQ)
	ON_BN_CLICKED(IDC_BROWSEEXE, OnBrowseEXE)
	ON_BN_CLICKED(IDC_BROWSEICON, OnBrowseIcon)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage1 message handlers

BOOL CSEMPQWizardPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// Set the limit on the length of the mod name, and load settings
	m_ctlCustomName.SetLimitText(31);
	m_ctlCustomName.SetFocus();

	LoadSettings();

	// FALSE because we set the focus
	return FALSE;
}

void CSEMPQWizardPage1::OnDestroy() 
{
	CPropertyPage::OnDestroy();

	// If there was an icon selected, we need to destroy it here
	HICON hIcon = m_ctlIconDisplay.GetIcon();
	if (hIcon)
		DestroyIcon(hIcon);
}

void CSEMPQWizardPage1::OnBrowseMPQ() 
{
	// Pick the default directory to use in the open dialog.
	char szMPQDir[MAX_PATH + 1];

	// If there's an MPQ selected already, derive the path from that. If that 
	// doesn't work, use the MPQDraft startup directory.
	if (m_ctlMPQName.GetWindowText(szMPQDir, MAX_PATH))
	{
		PathRemoveFileSpec(szMPQDir);

		if (!PathIsDirectory(szMPQDir))
			strcpy(szMPQDir, theApp.GetStartupPath());
	}
	else
		strcpy(szMPQDir, theApp.GetStartupPath());

	// Set up all the other parameters of the open dialog and run it
	CString strFilter, strTitle;
	strFilter.LoadString(IDS_OPENMPQSETTINGS);
	strTitle.LoadString(IDS_OPENSEMPQMPQ);

	CFileDialog dlg(TRUE, NULL, NULL, OFN_EXPLORER | 
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, 
		strFilter);
	dlg.m_ofn.lpstrTitle = strTitle;
	dlg.m_ofn.lpstrInitialDir = szMPQDir;

	// Update the MPQ path if we have a new one
	if (dlg.DoModal() == IDOK)
		m_ctlMPQName.SetWindowText(dlg.GetPathName());
}

void CSEMPQWizardPage1::OnBrowseEXE() 
{
	// Get the initial directory from the previous path
	char szSEMPQDir[MAX_PATH + 1];
	if (m_ctlEXEName.GetWindowText(szSEMPQDir, MAX_PATH))
	{
		PathRemoveFileSpec(szSEMPQDir);

		if (!PathIsDirectory(szSEMPQDir))
			strcpy(szSEMPQDir, theApp.GetStartupPath());
	}
	else
		strcpy(szSEMPQDir, theApp.GetStartupPath());

	// Do the save dialog
	CString strDefExt, strFilter, strTitle;
	strDefExt.LoadString(IDS_DEFEXEEXT);
	strFilter.LoadString(IDS_OPENEXESETTINGS);
	strTitle.LoadString(IDS_SAVESEMPQ);

	CFileDialog dlg(FALSE, strDefExt, NULL, 
		OFN_EXPLORER | OFN_OVERWRITEPROMPT, strFilter);
	dlg.m_ofn.lpstrTitle = strTitle;
	dlg.m_ofn.lpstrInitialDir = szSEMPQDir;

	// Update the EXE path if successful
	if (dlg.DoModal() == IDOK)
		m_ctlEXEName.SetWindowText(dlg.GetPathName());
}

void CSEMPQWizardPage1::OnBrowseIcon() 
{
	/*
	// One more time for good measure
	// Get the initial directory
	CString strIconDir;
	m_ctlIconName.GetWindowText(strIconDir);
	if (strIconDir.GetLength())
	{
		int nDirLength = strIconDir.ReverseFind('\\');
		if (nDirLength != -1)
			strIconDir = strIconDir.Left(nDirLength);

		if (!PathIsDirectory(strIconDir))
			strIconDir = theApp.GetStartupPath();
	}
	else
		strIconDir = theApp.GetStartupPath();

	// Do the open dialog
	CString strFilter, strTitle;
	strFilter.LoadString(IDS_OPENICONSETTINGS);
	strTitle.LoadString(IDS_SELECTSEMPQICON);

	CFileDialog dlg(TRUE, NULL, NULL, OFN_EXPLORER | OFN_FILEMUSTEXIST | 
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, strFilter);
	dlg.m_ofn.lpstrTitle = strTitle;
	dlg.m_ofn.lpstrInitialDir = strIconDir;

	if (dlg.DoModal() == IDOK)
	{
		// Fetch the icon to use using windows shell functions
		SHFILEINFO shfi;
		HICON hOldIcon;
		CString strIconName = dlg.GetPathName();

		m_ctlIconName.SetWindowText(dlg.GetPathName());

		ZeroMemory(&shfi, sizeof(SHFILEINFO));
		SHGetFileInfo(strIconName, 0, &shfi, sizeof(shfi), SHGFI_ICON  | SHGFI_LARGEICON);

		hOldIcon = m_ctlIconDisplay.SetIcon(shfi.hIcon);
		if (hOldIcon)
			DestroyIcon(hOldIcon);
	}
*/	
}

BOOL CSEMPQWizardPage1::SaveSettings()
{
	// Load all the value name strings
	CString strRegSEMPQHdr, strRegCustomName, strRegMPQPath, 
		strRegSEMPQPath, strRegIconPath;
	strRegSEMPQHdr.LoadString(IDS_SEMPQKEY);
	strRegCustomName.LoadString(IDS_SEMPQNAMEKEY);
	strRegMPQPath.LoadString(IDS_SRCPATHKEY);
	strRegSEMPQPath.LoadString(IDS_SEMPQPATHKEY);
	strRegIconPath.LoadString(IDS_ICONPATHKEY);

	CString strCustomName, strMPQPath, strSEMPQPath, strIconPath;

	// Save it all
	m_ctlCustomName.GetWindowText(strCustomName);
	m_ctlMPQName.GetWindowText(strMPQPath);
	m_ctlEXEName.GetWindowText(strSEMPQPath);
	m_ctlIconName.GetWindowText(strIconPath);

	theApp.WriteProfileString(strRegSEMPQHdr, strRegCustomName, strCustomName);
	theApp.WriteProfileString(strRegSEMPQHdr, strRegMPQPath, strMPQPath);
	theApp.WriteProfileString(strRegSEMPQHdr, strRegSEMPQPath, strSEMPQPath);
	//theApp.WriteProfileString(strRegSEMPQHdr, strRegIconPath, strIconPath);

	return TRUE;
}

BOOL CSEMPQWizardPage1::LoadSettings()
{
	// Load all the value name strings
	CString strRegSEMPQHdr, strRegCustomName, strRegMPQPath, 
		strRegSEMPQPath, strRegIconPath;
	strRegSEMPQHdr.LoadString(IDS_SEMPQKEY);
	strRegCustomName.LoadString(IDS_SEMPQNAMEKEY);
	strRegMPQPath.LoadString(IDS_SRCPATHKEY);
	strRegSEMPQPath.LoadString(IDS_SEMPQPATHKEY);
	strRegIconPath.LoadString(IDS_ICONPATHKEY);

	// Load the values
	CString strCustomName = theApp.GetProfileString(strRegSEMPQHdr, strRegCustomName),
		strMPQPath = theApp.GetProfileString(strRegSEMPQHdr, strRegMPQPath),
		strSEMPQPath = theApp.GetProfileString(strRegSEMPQHdr, strRegSEMPQPath),
		strIconPath = theApp.GetProfileString(strRegSEMPQHdr, strRegIconPath);

	// Do a sanity check on the values we got from the registry, then set them
	if (strCustomName.GetLength() < 32)
		m_ctlCustomName.SetWindowText(strCustomName);
	if (PathFileExists(strMPQPath))
		m_ctlMPQName.SetWindowText(strMPQPath);
	m_ctlEXEName.SetWindowText(strSEMPQPath);
	/*if (PathFileExists(strIconPath))
		m_ctlIconName = strIconPath;*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage2 property page

IMPLEMENT_DYNCREATE(CSEMPQWizardPage2, CPatchTargetPage)

CSEMPQWizardPage2::CSEMPQWizardPage2()
	: CPatchTargetPage(TRUE, IDS_SEMPQWIZARD2TITLE, IDS_SEMPQWIZARD2INFO)
{
	//{{AFX_DATA_INIT(CSEMPQWizardPage2)
	//}}AFX_DATA_INIT
}

CSEMPQWizardPage2::~CSEMPQWizardPage2()
{
}

BOOL CSEMPQWizardPage2::OnSetActive() 
{
	// Set the buttons how we want them and remove the help button
	CSEMPQWizard *pWizard = (CSEMPQWizard *)GetParent();

	pWizard->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	pWizard->ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	return CPropertyPage::OnSetActive();
}

BEGIN_MESSAGE_MAP(CSEMPQWizardPage2, CPatchTargetPage)
	//{{AFX_MSG_MAP(CSEMPQWizardPage2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage2 message handlers

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage3 property page

IMPLEMENT_DYNCREATE(CSEMPQWizardPage3, CPropertyPage)

CSEMPQWizardPage3::CSEMPQWizardPage3() 
	: CPluginPage(TRUE, IDS_SEMPQWIZARD3TITLE)
{
	//{{AFX_DATA_INIT(CSEMPQWizardPage3)
	//}}AFX_DATA_INIT
}

CSEMPQWizardPage3::~CSEMPQWizardPage3()
{
}

BEGIN_MESSAGE_MAP(CSEMPQWizardPage3, CPluginPage)
	//{{AFX_MSG_MAP(CSEMPQWizardPage3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizardPage3 message handlers

BOOL CSEMPQWizardPage3::OnSetActive() 
{
	// Set the wizard buttons
	CSEMPQWizard *pWizard = (CSEMPQWizard *)GetParent();
	pWizard->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	
	// Set the patch target and list plugins if necessary
	if (SetPatchTarget(""))
		ListPlugins();

	return CPropertyPage::OnSetActive();
}

BOOL CSEMPQWizardPage3::OnWizardFinish() 
{
	// Verify that this page has valid data
	if (!OnKillActive())
		return FALSE;

	// Verify that all plugins are ready
	if (!PluginsReady())
		return FALSE;

	// Do it
	if (!((CSEMPQWizard *)GetParent())->CreateSEMPQ())
		return FALSE;

	return CPropertyPage::OnWizardFinish();
}

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizard

IMPLEMENT_DYNAMIC(CSEMPQWizard, CPropertySheet)

CSEMPQWizard::CSEMPQWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_firstPage);
	AddPage(&m_secondPage);
	AddPage(&m_thirdPage);

	SetWizardMode();
}

CSEMPQWizard::CSEMPQWizard(LPCTSTR pszCaption, 
	CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_firstPage);
	AddPage(&m_secondPage);
	AddPage(&m_thirdPage);

	SetWizardMode();
}

CSEMPQWizard::~CSEMPQWizard()
{
}

BOOL CSEMPQWizard::CreateSEMPQ()
{
	// First of all, save the settings
	m_firstPage.SaveSettings();
	m_secondPage.SaveSettings();
	m_thirdPage.SaveSettings();

	CString strEXEName, strMPQName, strSEMPQError, strMessage;

	// Load the error string just in case
	strSEMPQError.LoadString(IDS_SEMPQFAILED);

	// Get the data from the first page
	//HICON hIcon = m_firstPage.m_ctlIconDisplay.GetIcon();
	/*if (!hIcon)
	{
		hIcon = theApp.LoadIcon(IDR_MAINFRAME);
		if (!hIcon)
		{
			strMessage.LoadString(IDS_CANTLOADDEFICON);
			MessageBox(strMessage, szSEMPQError, MB_OK | MB_ICONSTOP);

			return;
		}
	}*/

	m_firstPage.GetSEMPQFileName(strEXEName);
	m_firstPage.GetMPQFileName(strMPQName);

	// Verify the MPQ path is correct
	if (!PathFileExists(strMPQName))
	{
		strMessage.Format(IDS_BADMPQ, strMPQName);
		MessageBox(strMessage, strSEMPQError, MB_OK | MB_ICONSTOP);

		return FALSE;
	}

	// Create the progress dialog and set the basic settings
	CProgressWnd wndProgress;
	CString strSEMPQCaption;

	strSEMPQCaption.LoadString(IDS_SEMPQCAPTION);
	if (!wndProgress.Create(this, strSEMPQCaption))
	{
		strMessage.LoadString(IDS_WINDOWSERROR);
		MessageBox(strMessage, strSEMPQError, MB_ICONSTOP | MB_OK);

		return FALSE;
	}
	
	wndProgress.SetWindowSize(2);
	wndProgress.SetRange(0, 100);

	// Get the list of plugin modules to include (from the third page)
	CPluginPage::PluginModuleList modules;
	if (!GetPluginModules(modules))
	{
		// Failed to get them
		strMessage = strSEMPQError;
		MessageBox(strMessage, strSEMPQError, MB_OK | MB_ICONSTOP);

		return FALSE;
	}

	// Build SEMPQCreationParams from wizard pages
	SEMPQCreationParams params;

	// Get the custom name for the SEMPQ
	CString strCustomName;
	m_firstPage.GetCustomName(strCustomName);

	params.outputPath = std::string((LPCSTR)strEXEName);
	params.sempqName = std::string((LPCSTR)strCustomName);
	params.mpqPath = std::string((LPCSTR)strMPQName);
	params.iconPath = "";  // Icon path not implemented yet

	// Get the second page info we need
	const PROGRAMENTRY *lpProgram = m_secondPage.GetSelectedProgram();
	const PROGRAMFILEENTRY *lpFile = m_secondPage.GetSelectedFile();
	CString strProgramPath, strParameters;
	m_secondPage.GetProgramPath(strProgramPath);
	m_secondPage.GetParameters(strParameters);

	// Compile the flags
	DWORD dwFlags = 0;
	if (m_secondPage.RedirOpenFileEx())
		dwFlags |= MPQD_EXTENDED_REDIR;
	params.flags = dwFlags;
	params.parameters = std::string((LPCSTR)strParameters);

	// Are we using a supported app (registry-based), or a custom one (path-based)?
	if (lpProgram && lpFile)
	{
		// A built-in one (registry-based)
		params.useRegistry = true;
		params.registryKey = std::string(lpProgram->szRegistryKey);
		params.registryValue = std::string(lpProgram->szRegistryValue);
		params.valueIsFullPath = false;
		params.targetFileName = std::string(lpFile->szTargetFileName);
		params.spawnFileName = std::string(lpFile->szFileName);
		params.shuntCount = lpFile->nShuntCount;
	}
	else
	{
		// Custom one (path-based)
		params.useRegistry = false;
		params.targetPath = std::string((LPCSTR)strProgramPath);
		// Extract filename from path
		CString strFileName = strProgramPath;
		PathStripPath(strFileName.GetBuffer());
		strFileName.ReleaseBuffer();
		params.targetFileName = std::string((LPCSTR)strFileName);
		params.spawnFileName = params.targetFileName;
		params.shuntCount = 0;
	}

	// Copy plugin modules (with full metadata including component/module IDs)
	for (INT_PTR i = 0; i < modules.GetSize(); i++)
	{
		params.pluginModules.push_back(modules.GetAt(i));
	}

	// Create the SEMPQ using the modernized creator
	SEMPQCreator creator;

	// Progress callback adapter
	auto progressCallback = [&wndProgress](int progress, const std::string& statusText) {
		wndProgress.SetPos(progress);
		wndProgress.SetText(statusText.c_str());
	};

	std::string errorMessage;
	bool bSuccess = creator.createSEMPQ(params, progressCallback, nullptr, errorMessage);

	::MessageBeep(MB_ICONEXCLAMATION);
	if (!bSuccess)
	{
		::DeleteFile(strEXEName);
		if (!errorMessage.empty())
		{
			MessageBox(errorMessage.c_str(), strSEMPQError, MB_OK | MB_ICONSTOP);
		}
	}

	return bSuccess ? TRUE : FALSE;
}


BOOL CSEMPQWizard::GetPluginModules(CPluginPage::PluginModuleList &modules)
{
	// This is pretty straightforward, as most of the work is done by 
	// the plugin page. All we need to do is add the patcher DLL to the list 
	// so that it will be packed into the SEMPQ.
	modules.RemoveAll();

	// First, get the plugin modules from the plugin page
	if (!m_thirdPage.GetSelectedPluginsAndModules(NULL, modules))
		return FALSE;	// Seems like a plugin wasn't ready

	// Get patcher DLL path
	CString *lpstrPatcherDLL = theApp.GetPatcherDLLPath();
	if (!lpstrPatcherDLL)
		return FALSE;

	// And add it to the list
	MPQDRAFTPLUGINMODULE dllModule;
	dllModule.dwComponentID = MPQDRAFT_COMPONENT;
	dllModule.dwModuleID = MPQDRAFTDLL_MODULE;
	dllModule.bExecute = FALSE;
	strcpy(dllModule.szModuleFileName, *lpstrPatcherDLL);

	modules.Add(dllModule);

	// Done
	return TRUE;
}

BEGIN_MESSAGE_MAP(CSEMPQWizard, CPropertySheet)
	//{{AFX_MSG_MAP(CSEMPQWizard)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEMPQWizard message handlers
