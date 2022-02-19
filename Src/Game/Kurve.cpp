#include "KurveStd.h"
#include "KurveApp.h"

#include "KurveView.h"
#include "KurveGame.h"
#include "KurveResources.h"

// audio lib
#pragma comment(lib, "ZFXAudio.lib")

KurveApp g_KurveApp;



INT WINAPI wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
	return Game(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

//========================================================================
//
// Game App
//
//========================================================================

BaseGameLogic* KurveApp::VCreateGameAndView() {
	BaseGameLogic *game = NULL;


	game = K_new KurveGame();

	RegisterGameSpecificEvents();

	gActorMgr->SetFactory(K_new ActorFactory());


	// Views
//	game->VAddView(shared_ptr<IGameView>(K_new MainMenuView()));

	return game;
}


// Register all game events
void KurveApp::RegisterGameSpecificEvents() {

}


HICON KurveApp::VGetIcon()
{
	return LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
}