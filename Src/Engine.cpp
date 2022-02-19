#include "EngineStd.h"
#include "GameApp.h"

int WINAPI Game(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPWSTR lpCmdLine,
				   int nCmdShow)
{
	// Checks for Memory Leaks
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpFlag);


	/*DXUTSetCallbackD3D9DeviceCreated( GameApp::OnCreateDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( GameApp::OnDestroyDevice );
	DXUTSetCallbackMsgProc( GameApp::MsgProc );
    DXUTSetCallbackD3D9DeviceReset( GameApp::OnResetDevice );
    DXUTSetCallbackD3D9DeviceLost( GameApp::OnLostDevice );
	DXUTSetCallbackD3D9FrameRender( GameApp::OnRender );
	DXUTSetCallbackFrameMove( GameApp::OnUpdateGame );	*/

	try {
	if(!g_pApp->Init(hInstance) )
		return false;

	g_pApp->Go();
	} catch(Ogre::Exception &e) {
		const int buffersize = 512;
		wchar_t buffer[buffersize];
		MultiByteToWideChar(CP_ACP, 0, e.getFullDescription().c_str(), -1, buffer, buffersize);
		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}

	/*int y = _CrtDumpMemoryLeaks();
	char buffer[100];
	sprintf(buffer, "Leaks?: %i", y);
	MessageBoxA(NULL, buffer, "Leaks", MB_OK);*/

	return 0;
}