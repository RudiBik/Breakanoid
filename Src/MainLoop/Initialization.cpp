#include "..\EngineStd.h"

#include "Initialization.h"

#include <shlobj.h>
#include <direct.h>


void CheckHardDisk(const DWORDLONG diskSpaceNeeded)
{
	// Check for enough free disk space on the current disk.
	int const drive = _getdrive();
	struct _diskfree_t diskfree;

	_getdiskfree(drive, &diskfree);

	unsigned __int64 const neededClusters = 
		diskSpaceNeeded /(diskfree.sectors_per_cluster*diskfree.bytes_per_sector);

	if (diskfree.avail_clusters < neededClusters)
	{
		// if you get here you don’t have enough disk space!
		MessageBox(NULL, "error", "Not enough Disk space", MB_OK);
	}
}


void CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded)
{
	MEMORYSTATUSEX status;
	GlobalMemoryStatusEx(&status);
	if (status.ullTotalPhys < (physicalRAMNeeded))
	{
		// you don’t have enough physical memory. Tell the player to go get a real 
		// computer and give this one to his mother. 
		MessageBox(NULL, "error", "Not enough physical memory", MB_OK);
	}

	// Check for enough free memory.
	if (status.ullAvailVirtual < virtualRAMNeeded)
	{
		// you don’t have enough virtual memory available. 
		// Tell the player to shut down the copy of Visual Studio running in the
		// background, or whatever seems to be sucking the memory dry.
		
		MessageBox(NULL, "error", "Not enough virtual memory", MB_OK);
	}

	char *buff = K_new char[(unsigned long)virtualRAMNeeded];
	if (buff) {
		delete[] buff;
		buff = NULL;
	}
	else
	{
		// The system lied to you. When you attempted to grab a block as big
		// as you need the system failed to do so. Something else is eating 
		// memory in the background; tell them to shut down all other apps 
		// and concentrate on your game.
		
		MessageBox(NULL, "error", "Not enough vmemory, quit some applications", MB_OK);
	}
}


DWORD GetFreeVRAM()
{
/***
	// NOTE: This method is deprecated, and unfortunately not really replaced with
	// anything useful.....


	DDSCAPS2 ddsCaps;
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
		
	ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	DWORD dwUsedVRAM = 0;
	DWORD dwTotal=0;
	DWORD dwFree=0;

	// lp_DD points to the IDirectDraw object	
	HRESULT hr = g_pDisplay->GetDirectDraw()->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree); 
		
	// dwUsedVRAM holds the number of bytes of VRAM used
	dwUsedVRAM = dwTotal-dwFree;
	return dwUsedVRAM;								
****/
	return 0;
}








bool IsOnlyInstance(LPCWSTR gameTitle)
{
	// Find the window.  If active, set and return false
	// Only one game instance may have this mutex at a time...

	HANDLE handle = CreateMutexW(NULL, TRUE, gameTitle);

	// Does anyone else think 'ERROR_SUCCESS' is a bit of a dichotomy?
	if (GetLastError() != ERROR_SUCCESS)
	{
		HWND hWnd = FindWindowW(gameTitle, NULL);
		if (hWnd) 
		{
			// An instance of your game is already running.
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
			return false;
		}
	}
	return true;
}

//
// GetSaveGameDirectory - Chapter 5 - page 146

const TCHAR *GetSaveGameDirectory(HWND hWnd, const TCHAR *gameAppDirectory)
{
	HRESULT hr;

	static TCHAR mSaveGameDirectory[MAX_PATH];
	TCHAR userDataPath[MAX_PATH];

	hr = SHGetSpecialFolderPath(hWnd, userDataPath, CSIDL_APPDATA, true);

#ifdef _VS2005_
	_tcscpy_s(mSaveGameDirectory, userDataPath);
	_tcscat_s(mSaveGameDirectory, _T("\\"));
	_tcscat_s(mSaveGameDirectory, gameAppDirectory);
#else
	_tcscpy(mSaveGameDirectory, userDataPath);
	_tcscat(mSaveGameDirectory, _T("\\"));
	_tcscat(mSaveGameDirectory, gameAppDirectory);
#endif


	// Does our directory exist?
	if (0xffffffff == GetFileAttributes(mSaveGameDirectory))
	{
		// Nope - we have to go make a new directory to store application data.
		//
		// On Win32 systems you could call SHCreateDirectoryEx to create an
		// entire directory tree, but this code is included for ease of portability to other 
		// systems without that.
		//
		// Game Coding Complete reference - Chapter 11, page 388
		//
		TCHAR current[MAX_PATH];
		TCHAR myAppData[MAX_PATH];

#ifdef _VS2005_
		_tcscpy_s(current, userDataPath);
		_tcscpy_s(myAppData, gameAppDirectory);
#else
		_tcscpy(current, userDataPath);
		_tcscpy(myAppData, gameAppDirectory);
#endif

		TCHAR token[MAX_PATH];
		token[0] = 0;

		do {
			TCHAR *left = _tcschr(myAppData, '\\');
			if (left==NULL)
			{
				#ifdef _VS2005_
					_tcscpy_s(token, myAppData);
				#else
					_tcscpy(token, myAppData);
				#endif
				myAppData[0] = 0;
			}
			else
			{
				#ifdef _VS2005_
					_tcsncpy_s(token, myAppData, left-myAppData);
					token[left-myAppData] = 0;
					_tcscpy_s(myAppData, left+1);
				#else
					_tcsncpy(token, myAppData, left-myAppData);
					token[left-myAppData] = 0;
					_tcscpy(myAppData, left+1);
				#endif
			}

			if (_tcslen(token))
			{

				#ifdef _VS2005_
					_tcscat_s(current, _T("\\"));
					_tcscat_s(current, token);
				#else
					_tcscat(current, _T("\\"));
					_tcscat(current, token);
				#endif
				if (false == CreateDirectory(current, NULL))
				{
					int error = GetLastError();
					if (error != ERROR_ALREADY_EXISTS)
					{
						return false;
					}
				}
			}

		} while (_tcslen(myAppData));
	}
		
#ifdef _VS2005_
	_tcscat_s(mSaveGameDirectory, _T("\\"));
#else
	_tcscat(mSaveGameDirectory, _T("\\"));
#endif
	return mSaveGameDirectory;
}


