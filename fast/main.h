#pragma once
/* Contents of file listplug.h */


typedef struct {
  int size;
  DWORD PluginInterfaceVersionLow;
  DWORD PluginInterfaceVersionHi;
  char DefaultIniName[MAX_PATH];
} ListDefaultParamStruct;

HWND __stdcall ListLoad(HWND ParentWin, char* FileToLoad, int ShowFlags);
void __stdcall ListCloseWindow(HWND ListWin);
void __stdcall ListGetDetectString(char* DetectString, int maxlen);
int __stdcall ListSearchText(HWND ListWin,
                             char* SearchString,
                             int SearchParameter);
int __stdcall ListSendCommand(HWND ListWin, int Command, int Parameter);
int __stdcall ListPrint(HWND ListWin,
                        char* FileToPrint,
                        char* DefPrinter,
                        int PrintFlags,
                        RECT* Margins);
int __stdcall ListNotificationReceived(HWND ListWin,
                                       int Message,
                                       WPARAM wParam,
                                       LPARAM lParam);
void __stdcall ListSetDefaultParams(ListDefaultParamStruct* dps);
HBITMAP __stdcall ListGetPreviewBitmap(char* FileToLoad,
                                       int width,
                                       int height,
                                       char* contentbuf,
                                       int contentbuflen);