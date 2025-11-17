// Minimal afxres.h replacement for MinGW compilation
// This file provides basic definitions needed for resource compilation
// without requiring MFC

#ifndef _AFXRES_H
#define _AFXRES_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

// MFC resource definitions that might be used in .rc files
#define IDC_STATIC              (-1)

#endif // _AFXRES_H

