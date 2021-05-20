#pragma once

#define WIN32_LEAN_AND_MEAN  // 从 Windows 头文件中排除极少使用的内容
#include <windows.h>

#include <commdlg.h>
#include <malloc.h>
#include <math.h>
#include <richedit.h>
#include <shellapi.h>
#include <stdlib.h>

#include <Shlwapi.h>
#include <atlbase.h>
#include <atlfile.h>
#include <atlwin.h>

#include <string>

#define LC_COPY 1
#define LC_NEWPARAMS 2
#define LC_SELECT_ALL 3
#define LC_SET_PERCENT 4

#define LCP_WRAP_TEXT 1
#define LCP_FIT_TO_WINDOW 2
#define LCP_ANSI 4
#define LCP_ASCII 8
#define LCP_VARIABLE 12
#define LCP_FORCE_SHOW 16

#define LCS_FIND_FIRST 1
#define LCS_MATCH_CASE 2
#define LCS_WHOLE_WORDS 4
#define LCS_BACK_WARDS 8

#define ITM_PERCENT 0xFFFE
#define ITM_FONT_STYLE 0xFFFD
#define ITM_WRAP 0xFFFC
#define ITM_FIT 0xFFFB
#define ITM_NEXT 0xFFFA

#define LISTPLUGIN_OK 0
#define LISTPLUGIN_ERROR 1