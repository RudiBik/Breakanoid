#pragma once

#include "MainLoop\CProcess.h"
#include "EventManager\EventManager.h"

#include <OGRE/OgreConfigFile.h>
#include <CEGUI.h>
#include <RendererModules\Ogre\CEGUIOgreRenderer.h>
#include "MovementController.h"
#include "Actors\Actors.h"


class BaseGameLogic;
class EventManager;

#define QUIT_NO_PROMPT MAKELPARAM(-1,-1)
#define MSG_END_MODAL (WM_USER+100)

enum
{
	// Lower numbers get drawn first
	ZORDER_BACKGROUND, 
	ZORDER_LAYER1, 
	ZORDER_LAYER2, 
	ZORDER_LAYER3, 
	ZORDER_TOOLTIP
};

class GameApp : public OIS::KeyListener, public OIS::MouseListener {
protected:
	bool		mbRunning;

public:
	// audio
	LPZFXAUDIO       mpAudio;
	LPZFXAUDIODEVICE mpDevice;

	// sounds
	UINT		mSoundBack;
	UINT		mSoundPaddle;
	UINT		mSoundWall;
	UINT		mSoundBlock[3];
	UINT		mSoundWin;

	// free space in eventbuffers
	float mfFreeSpaceEventStack;
	HINSTANCE	mhInst;
	HWND		mhWnd;

	BaseGameLogic	   *mpGame;
	EventManager	   *mpEventMgr;

	// CEGUI
	CEGUI::OgreRenderer *mpCEGUIRenderer;
	CEGUI::System		*mpCEGUISystem;

	TextRenderer		*mpTextRenderer;

	Ogre::Root		   *mpRoot;
	Ogre::RenderWindow *mpWindow;
	MovementController *mpInput;
	Ogre::String		msPlugins;
	Ogre::String		msResources;
	Ogre::String		msConfig;

	GameApp();

	virtual WCHAR *VGetGameTitle()=0;
	virtual WCHAR *VGetGameDirectory()=0;
	virtual HICON  VGetIcon()=0;

	virtual bool Init(HINSTANCE	hInst);
	void RegisterBaseGameEvents();

	LRESULT OnClose();

	void Go();

	virtual BaseGameLogic* VCreateGameAndView()=0;

	// Running/Quitting
	void AbortGame() { mbRunning = false; }
	bool IsRunning() { return mbRunning; }

	DWORD GetWidth() { return mpWindow->getWidth(); }
	DWORD GetHeight() { return mpWindow->getHeight(); }

	// the movementcontroller registers the gameapp class for key events
    virtual bool keyPressed( const OIS::KeyEvent& evt );
    virtual bool keyReleased( const OIS::KeyEvent& evt );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent& evt );
    virtual bool mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id );
};

// GameStates
enum BaseGameState {
	BGS_Initializing,
	BGS_LoadingEnvironment,
	BGS_MainMenu,
	BGS_SpawnAI,
	BGS_Running,
	BGS_Paused,
	BGS_Menu,
	BGS_Lose,
	BGS_Won
};


class BaseGameLogic : public IGameLogic {
	friend class GameApp;

protected:
	CProcessManager *mpProcessManager;

	GameViewList	mGameViews;

	BaseGameState	mState;
	int				mExpectedPlayers;
	bool			mbRenderDiagnostics;
	bool			mbPaused;

public:
	CRandom			 mRandom;

	BaseGameLogic();
	virtual ~BaseGameLogic();

	virtual void VClear();
	virtual void VDestroy();

	CRandom* VGetRandom() { return &mRandom; }

	void TogglePause();
	bool IsPaused() { return mbPaused; }

	virtual void VAddView(IGameView *pView, ActorId aid = -1);
	virtual void VRemoveView(IGameView *pView);
	virtual void VRemoveAllViews();
	virtual HActor VAddActor(char *chName, int type);
	virtual IActor* const VGetActor(HActor &actor);
	//virtual ActorId VGetRandomActor(ActorId ignore);
	virtual void VRemoveActor(HActor &actor);
	//virtual void VMoveActor(ActorId id, EMatrix *mat); 
	
	virtual bool VLoadGame(std::string gamename) { return true; }
	virtual void VBuildInitialScene() {}

	virtual void VOnUpdate();
	virtual void VChangeState(BaseGameState newState);
	virtual void VAttachProcess(shared_ptr<CProcess> p) { if(mpProcessManager) {mpProcessManager->Attach(p);}}

	virtual void VRenderDiagnostics();
	virtual IGamePhysics* VGetGamePhysics() { return NULL; }
};
extern GameApp *g_pApp;



//				HumanView
class HumanView : public IGameView {
protected:
	GameViewId		mId;
	ActorId			mAid;															

	CProcessManager		*mpProcessManager;	

public:
	HumanView();
	virtual ~HumanView();

	virtual void VOnUpdate(int deltaMilli);

	virtual void VBuildInitialScene() {}

	virtual GameViewType VGetType() { return GVT_Human; }
	virtual GameViewId VGetId() { return mId; }
	virtual void VOnAttach(GameViewId id, ActorId aid) { mId = id; mAid = aid; VBuildInitialScene(); }
};

