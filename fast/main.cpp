#include "pch.h"
#include "main.h"

HINSTANCE hInstance = nullptr;
HMODULE FLibHandle = 0;

char PARSE_FUNCTION[] = "force | (ext=\"VBK\")";

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      hInstance = (HINSTANCE)hModule;
      break;
    case DLL_PROCESS_DETACH:
      if (FLibHandle)
        FreeLibrary(FLibHandle);
      FLibHandle = NULL;
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
  }
  return TRUE;
}

char* strlcpy(char* p, char* p2, int maxlen) {
  if ((int)strlen(p2) >= maxlen) {
    strncpy(p, p2, maxlen);
    p[maxlen] = 0;
  } else
    strcpy(p, p2);
  return p;
}

void __stdcall ListGetDetectString(char* DetectString, int maxlen) {
  strlcpy(DetectString, PARSE_FUNCTION, maxlen);
}

int lastloadtime = 0;  // Workaround to RichEdit bug

int __stdcall ListNotificationReceived(HWND ListWin,
                                       int Message,
                                       WPARAM wParam,
                                       LPARAM lParam) {
  switch (Message) {
    case WM_COMMAND:
      if (HIWORD(wParam) == EN_UPDATE &&
          abs((long)(GetCurrentTime() - lastloadtime)) > 1000) {
        int firstvisible = SendMessage(ListWin, EM_GETFIRSTVISIBLELINE, 0, 0);
        int linecount = SendMessage(ListWin, EM_GETLINECOUNT, 0, 0);
        if (linecount > 0) {
          int percent = MulDiv(firstvisible, 100, linecount);
          PostMessage(GetParent(ListWin), WM_COMMAND,
                      MAKELONG(percent, ITM_PERCENT), (LPARAM)ListWin);
        }
        return 0;
      }
      break;
    case WM_NOTIFY:
      break;
    case WM_MEASUREITEM:
      break;
    case WM_DRAWITEM:
      break;
  }
  return 0;
}

int ShowVBK(LPCWSTR FilePath,
            CWindow& RichEdit,
            CAtlFileMapping<char>& FileMapping);

struct {
  WCHAR Ext[100];
  int (*ShowContent)(LPCWSTR FilePath,
                     CWindow& RichEdit,
                     CAtlFileMapping<char>& FileMapping);
} FILE_MAPPING[] = {L".vbk", &ShowVBK};

int MapFileContent(LPCWSTR ext, LPCWSTR FilePath,
                   CWindow& RichEdit,
                   CAtlFileMapping<char>& FileMapping) {

  for (auto& v : FILE_MAPPING) {
    if (_wcsicmp(ext, v.Ext) == 0) {
      if (v.ShowContent(FilePath, RichEdit, FileMapping) != LISTPLUGIN_ERROR) {
        return LISTPLUGIN_OK;
      }
    }
  }
  return LISTPLUGIN_ERROR;
}

#define RICH_EDIT_STYLE                                              \
  (WS_CHILD | ES_MULTILINE | ES_READONLY | WS_HSCROLL | WS_VSCROLL | \
   ES_NOHIDESEL)

HWND __stdcall ListLoadW(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags) {
  auto ext = PathFindExtension(FileToLoad);
  if (ext == NULL) {
    return NULL;
  }
  if (ShowFlags & LCP_FORCE_SHOW == 0) {  // don't check extension in this case!
    for (auto& v : FILE_MAPPING) {
      if (_wcsicmp(ext, v.Ext) == 0) {
        ShowFlags = LCP_FORCE_SHOW;
        break;
      }
    }
    if (ShowFlags != LCP_FORCE_SHOW)
      return NULL;
  }

  CAtlFile file;
  if (FAILED(file.Create(FileToLoad, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) {
    return NULL;
  }

  if (!FLibHandle) {
    int OldError = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    FLibHandle = LoadLibrary(L"Riched20.dll");
    if ((int)FLibHandle < HINSTANCE_ERROR)
      FLibHandle = LoadLibrary(L"RICHED32.DLL");
    if ((int)FLibHandle < HINSTANCE_ERROR)
      FLibHandle = NULL;
    SetErrorMode(OldError);
  }
  CAtlFileMapping<char> FileMapping;
  if (FAILED(FileMapping.MapFile(file))) {
    return NULL;
  }

  RECT r;
  GetClientRect(ParentWin, &r);
  CWindow RichEdit;
  if (RichEdit.Create(L"RichEdit20A", ParentWin, r, L"", RICH_EDIT_STYLE)) {
    RichEdit.SendMessageW(EM_SETMARGINS, EC_LEFTMARGIN, 8);
    RichEdit.SendMessageW(EM_SETEVENTMASK, 0, ENM_UPDATE);
    PostMessage(ParentWin, WM_COMMAND, MAKELONG(LCP_ANSI, ITM_FONT_STYLE),
                (LPARAM)RichEdit.m_hWnd);
    if (MapFileContent(ext, FileToLoad, RichEdit, FileMapping) == LISTPLUGIN_ERROR) {
      return NULL;
    }
    RichEdit.ShowWindow(SW_SHOW);
  }
  return RichEdit.Detach();
}

HWND __stdcall ListLoad(HWND ParentWin, char* FileToLoad, int ShowFlags) {
  USES_CONVERSION;
  return ListLoadW(ParentWin, A2W(FileToLoad), ShowFlags);
}

int __stdcall ListLoadNextW(HWND ParentWin,
                            HWND ListWin,
                            WCHAR* FileToLoad,
                            int ShowFlags) {
  auto ext = PathFindExtension(FileToLoad);
  if (ext == NULL) {
    return LISTPLUGIN_ERROR;
  }

  if (ShowFlags & LCP_FORCE_SHOW == 0) {  // don't check extension in this case!
    for (auto& v : FILE_MAPPING) {
      if (_wcsicmp(ext, v.Ext) == 0) {
        ShowFlags = LCP_FORCE_SHOW;
        break;
      }
    }
    if (ShowFlags != LCP_FORCE_SHOW)
      return LISTPLUGIN_ERROR;
  }

  CAtlFile file;
  if (FAILED(file.Create(FileToLoad, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) {
    return LISTPLUGIN_ERROR;
  }
  CAtlFileMapping<char> FileMapping;
  if (FAILED(FileMapping.MapFile(file))) {
    return LISTPLUGIN_ERROR;
  }

  CWindow RichEdit(ListWin);
  RichEdit.SetWindowTextW(L"");
  RichEdit.SendMessageW(EM_SETMARGINS, EC_LEFTMARGIN, 8);
  RichEdit.SendMessageW(EM_SETEVENTMASK, 0, ENM_UPDATE);
  PostMessage(ParentWin, WM_COMMAND, MAKELONG(LCP_ANSI, ITM_FONT_STYLE),
              (LPARAM)ListWin);
  int result = MapFileContent(ext, FileToLoad, RichEdit, FileMapping);
  PostMessage(ParentWin, WM_COMMAND, MAKELONG(0, ITM_PERCENT), (LPARAM)ListWin);
  RichEdit.Detach();
  return result;
}

int __stdcall ListLoadNext(HWND ParentWin,
                           HWND ListWin,
                           char* FileToLoad,
                           int ShowFlags) {
  USES_CONVERSION;
  return ListLoadNextW(ParentWin, ListWin, A2W( FileToLoad), ShowFlags);
}

int __stdcall ListSendCommand(HWND ListWin, int Command, int Parameter) {
  switch (Command) {
    case LC_COPY:
      SendMessage(ListWin, WM_COPY, 0, 0);
      return LISTPLUGIN_OK;
    case LC_NEWPARAMS:
      PostMessage(GetParent(ListWin), WM_COMMAND, MAKELONG(0, ITM_NEXT), (LPARAM)ListWin);
      return LISTPLUGIN_ERROR;
    case LC_SELECT_ALL:
      SendMessage(ListWin, EM_SETSEL, 0, -1);
      return LISTPLUGIN_OK;
    case LC_SET_PERCENT:
      int firstvisible = SendMessage(ListWin, EM_GETFIRSTVISIBLELINE, 0, 0);
      int linecount = SendMessage(ListWin, EM_GETLINECOUNT, 0, 0);
      if (linecount > 0) {
        int pos = MulDiv(Parameter, linecount, 100);
        SendMessage(ListWin, EM_LINESCROLL, 0, pos - firstvisible);
        firstvisible = SendMessage(ListWin, EM_GETFIRSTVISIBLELINE, 0, 0);
        int firstchar = SendMessage(ListWin, EM_LINEINDEX, firstvisible, 0);
        SendMessage(ListWin, EM_SETSEL, firstchar, firstchar);
        pos = MulDiv(firstvisible, 100, linecount);
        PostMessage(GetParent(ListWin), WM_COMMAND, MAKELONG(pos, ITM_PERCENT),
                    (LPARAM)ListWin);
        return LISTPLUGIN_OK;
      }
      break;
  }
  return LISTPLUGIN_ERROR;
}

int _stdcall ListSearchText(HWND ListWin,
                            char* SearchString,
                            int SearchParameter) {
  FINDTEXTA find;
  int StartPos, Flags;

  if (SearchParameter & LCS_FIND_FIRST) {
    // Find first: Start at top visible line
    StartPos =
        SendMessage(ListWin, EM_LINEINDEX,
                    SendMessage(ListWin, EM_GETFIRSTVISIBLELINE, 0, 0), 0);
    SendMessage(ListWin, EM_SETSEL, StartPos, StartPos);
  } else {
    // Find next: Start at current selection+1
    SendMessage(ListWin, EM_GETSEL, (WPARAM)&StartPos, 0);
    StartPos += 1;
  }

  find.chrg.cpMin = StartPos;
  find.chrg.cpMax = SendMessage(ListWin, WM_GETTEXTLENGTH, 0, 0);
  Flags = 0;
  if (SearchParameter & LCS_WHOLE_WORDS)
    Flags |= FR_WHOLEWORD;
  if (SearchParameter & LCS_MATCH_CASE)
    Flags |= FR_MATCHCASE;
  if (!(SearchParameter & LCS_BACK_WARDS))
    Flags |= FR_DOWN;
  find.lpstrText = SearchString;
  int index = SendMessage(ListWin, EM_FINDTEXT, Flags, (LPARAM)&find);
  if (index != -1) {
    int indexend = index + strlen(SearchString);
    SendMessage(ListWin, EM_SETSEL, index, indexend);
    int line = SendMessage(ListWin, EM_LINEFROMCHAR, index, 0) - 3;
    if (line < 0)
      line = 0;
    line -= SendMessage(ListWin, EM_GETFIRSTVISIBLELINE, 0, 0);
    SendMessage(ListWin, EM_LINESCROLL, 0, line);
    return LISTPLUGIN_OK;
  } else {
    SendMessage(ListWin, EM_SETSEL, -1, -1);  // Restart search at the beginning
  }
  return LISTPLUGIN_ERROR;
}

void __stdcall ListCloseWindow(HWND ListWin) {
  DestroyWindow(ListWin);
}

int __stdcall ListPrint(HWND ListWin,
                        char* FileToPrint,
                        char* DefPrinter,
                        int PrintFlags,
                        RECT* Margins) {
  return LISTPLUGIN_ERROR;
}


HBITMAP __stdcall ListGetPreviewBitmapW(WCHAR* FileToLoad,
                                        int width,
                                        int height,
                                        char* contentbuf,
                                        int contentbuflen) {
  return NULL;
}

HBITMAP __stdcall ListGetPreviewBitmap(char* FileToLoad,
                                       int width,
                                       int height,
                                       char* contentbuf,
                                       int contentbuflen) {
  USES_CONVERSION;
  return ListGetPreviewBitmapW(A2W(FileToLoad), width, height, contentbuf,
                               contentbuflen);
}

int _stdcall ListSearchDialog(HWND ListWin, int FindNext) {
  /*	if (FindNext)
                  MessageBox(ListWin,"Find Next","test",0);
          else
                  MessageBox(ListWin,"Find First","test",0);*/
  return LISTPLUGIN_ERROR;  // use ListSearchText instead!
}
