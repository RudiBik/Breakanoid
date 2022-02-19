#pragma once

extern void CheckHardDisk(const DWORDLONG diskSpaceNeeded);
extern void CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded);
extern bool IsOnlyInstance(LPCWSTR gameTitle);
extern const TCHAR *GetSaveGameDirectory(HWND hWnd, const TCHAR *gameAppDirectory);

//extern DWORD GetFreeVRAM();		// deprecated!!!!
