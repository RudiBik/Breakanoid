#include "EngineStd.h"
#include "MainLoop\CProcess.h"

#include "GameApp.h"
#include "MainLoop\Initialization.h"
#include "Actors\Actors.h"
#include "EventManager\EventManagerImpl.h"
#include "EventManager\Events.h"

GameApp *g_pApp = NULL;


//				GAMEAPP
GameApp::GameApp() {
	g_pApp = this;
	mpGame = NULL;
	mpEventMgr = NULL;
	mfFreeSpaceEventStack = 0.0f;

	mpAudio = NULL;
	mpDevice = NULL;
	mpInput = NULL;
	mpRoot = NULL;
	mpWindow = NULL;
	msPlugins = Ogre::StringUtil::BLANK;
	msResources = Ogre::StringUtil::BLANK;
	msConfig = Ogre::StringUtil::BLANK;

	mbRunning = false;
}

//				INPUT
bool GameApp::keyPressed( const OIS::KeyEvent& evt ) {
	// inject events
	mpCEGUISystem->injectKeyDown(evt.key);
	mpCEGUISystem->injectChar(evt.text);

	return true;
}
bool GameApp::keyReleased( const OIS::KeyEvent& evt ) {
	
	// inject events
	mpCEGUISystem->injectKeyUp(evt.key);

	return true;
}

// Mouse events have to get converted before injecting into the cegui system
CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}

// OIS::MouseListener
bool GameApp::mouseMoved( const OIS::MouseEvent& evt ) {
	
	mpCEGUISystem->injectMouseMove((float)evt.state.X.rel, (float)evt.state.Y.rel);
	// Scroll wheel.
	if (evt.state.Z.rel)
		mpCEGUISystem->injectMouseWheelChange(evt.state.Z.rel / 120.0f);

	return true;
}

bool GameApp::mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id ) {
	
	mpCEGUISystem->injectMouseButtonDown(convertButton(id));

	return true;
}
bool GameApp::mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id ) {

	mpCEGUISystem->injectMouseButtonUp(convertButton(id));

	return true;
}


//				INIT
bool GameApp::Init(HINSTANCE	hInst) {

//Check for more instances of the game
#ifndef _DEBUG
	if(!IsOnlyInstance(VGetGameTitle()))
		return false;
#endif

	SetCursor(NULL);

// Check for adequate machine resources.

	mhInst = hInst;
	const DWORDLONG physicalRAM = 32 * MEGABYTE;
	const DWORDLONG virtualRAM = 64 * MEGABYTE;
	CheckMemory(physicalRAM, virtualRAM);

	const DWORDLONG diskSpace = 10 * MEGABYTE;
	CheckHardDisk(diskSpace);

	const int minCpuSpeed = 266;
	extern int GetCPUSpeed();
	int thisCPU = GetCPUSpeed();
	if (thisCPU < minCpuSpeed)
		MessageBox(NULL, "error", "Not enough CPU Speed... continuing", MB_OK);


	// Event Manager
	g_pEventStack = K_new DoubleBufferedAllocator();
	g_pEventStack->init(MEGABYTE * 0.01);
	mpEventMgr = K_new EventManager("GameApp Eventmanager", true);
	if(!mpEventMgr)
		return false;

	RegisterBaseGameEvents();

	shared_ptr<IEventListener> snooper = shared_ptr<IEventListener>(K_new EventSnooper("Log.txt"));                   //********
	safeAddListener(snooper, EventType(kpAllEventTypes));

	// Clock
	if(!Clock::Init()) {
		MessageBox(NULL, "g_Clock->Init: QueryPerformanceFrequency not supported!", "Error", MB_OK);
		return false;
	}
	g_pClock = K_new Clock(30.0f);


	// Init Ogre
	{
	#ifdef _DEBUG
		msPlugins = "plugins_d.cfg";
		msConfig = "ogre_d.cfg";
		msResources = "resources_d.cfg";
	#else
		msPlugins = "plugins.cfg";
		msConfig = "ogre.cfg";
		msResources = "resources.cfg";
	#endif

		mpRoot = new Ogre::Root(msPlugins, msConfig);

		// laod resources path from file, doesnt load them
		Ogre::ConfigFile cf;
		cf.load(msResources);

		// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
		Ogre::String secName, typeName, archName;
		while (seci.hasMoreElements())
		{
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); ++i)
			{
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}


		// Creating the Rendersystem, DirectX
		/*if(!(mpRoot->restoreConfig() || mpRoot->showConfigDialog())) {
			ERR("Couldn't find config file!");
			return false;
		} */
		mpRoot->restoreConfig();
		mpRoot->showConfigDialog();

		mpWindow = mpRoot->initialise(true, "Engine");
		mpWindow->setAutoUpdated(false);

		mpWindow->getCustomAttribute("WINDOW", &mhWnd);

		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}

	// Initialise CEGUI
	{
		mpCEGUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
		mpCEGUISystem = &CEGUI::System::getSingleton();

		// Add the resources
		CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
		CEGUI::Font::setDefaultResourceGroup("Fonts");
		CEGUI::Scheme::setDefaultResourceGroup("Schemes");
		CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
		CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

		// create the font
		CEGUI::FontManager::getSingleton().create("arialblk-10.font");

		// create the scheme
		CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
		CEGUI::SchemeManager::getSingleton().create("OgreTray.scheme");
		CEGUI::SchemeManager::getSingleton().create("VanillaSkin.scheme");
		CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
		CEGUI::System::getSingleton().setDefaultFont( "DejaVuSans-10" );
	}

	// Initialise TextRenderer
	{
		mpTextRenderer = K_new TextRenderer();
	}

	// audio/input
	{
		// audio
		mpAudio = new ZFXAudio(mhInst);
		if(!mpAudio)
			return false;

		if(FAILED(mpAudio->CreateDevice()))
			return false;
		mpDevice = mpAudio->GetDevice();
		if(!mpDevice)
			return false;

		char Buffer[512];

		GetCurrentDirectoryA( MAX_PATH-1, Buffer );
		strcat(Buffer, "\\Data\\Sounds");
		if(FAILED(mpDevice->Init(mhWnd, Buffer, true)))
			return false;

		// Load sounds
		mpDevice->LoadSound("back.wav", &mSoundBack);
		mSoundBack = 0;
		mpDevice->LoadSound("Sound6.wav", &mSoundPaddle);
		mSoundPaddle = 1;
		mpDevice->LoadSound("Sound8.wav", &mSoundWall);
		mSoundWall = 2;
		mpDevice->LoadSound("Sound9.wav", &mSoundBlock[0]);
		mSoundBlock[0] = 3;
		mpDevice->LoadSound("Sound10.wav", &mSoundBlock[1]);
		mSoundBlock[1] = 4;
		mpDevice->LoadSound("Sound11.wav", &mSoundBlock[2]);
		mSoundBlock[2] = 5;
		mpDevice->LoadSound("Sound12.wav", &mSoundWin);
		mSoundWin = 6;

		// create movementcontroller using OIS
		mpInput = K_new MovementController();
		mpInput->Init();
	}

	// GameLogic
	mpGame = VCreateGameAndView();
	if(!mpGame)
		return false;


	mbRunning = true;

	return true;
}

//				RegisterBaseGameEvents                                                                                              REGISTERBASEGAMEEVENTS
void GameApp::RegisterBaseGameEvents() {
	if(NULL == mpEventMgr) {
		assert(0 && "Cannot register events when the Eventmanager isnt initialized!");
		return;
	}

	mpEventMgr->RegisterEvent(EvtData_New_Game::sEventType);
	mpEventMgr->RegisterEvent(EvtData_Request_Start_Game::sEventType);
	mpEventMgr->RegisterEvent(EvtData_Game_Paused::sEventType);
	mpEventMgr->RegisterEvent(EvtData_Game_Resume::sEventType);
	mpEventMgr->RegisterEvent(EvtData_Error::sEventType);
	mpEventMgr->RegisterEvent(EvtData_GameState::sEventType);
	mpEventMgr->RegisterEvent(EventType(kpAllEventTypes));
}



//				OnClose
LRESULT GameApp::OnClose()
{
	// release all the game systems in reverse order from which they were created
	SAFE_DELETE(mpAudio);
	SAFE_DELETE(mpGame);
	SAFE_DELETE(g_pClock);
	SAFE_DELETE(mpEventMgr);
	SAFE_DELETE(g_pEventStack);
	gActorMgr->Del();
	SAFE_DELETE(mpInput);
	SAFE_DELETE(mpTextRenderer);
	CEGUI::OgreRenderer::destroySystem();
	mpWindow->destroy();
	mpWindow = NULL;
	SAFE_DELETE(mpRoot);

	return 0;
}


void GameApp::Go() {
	mpRoot->clearEventTimes();

	QueryPerformanceCounter((LARGE_INTEGER*)&g_pClock->mrealLast);
	while(mbRunning && !mpWindow->isClosed()) {
		// render
		mpWindow->update(false);
		mpRoot->renderOneFrame();
		g_pApp->mpGame->VRenderDiagnostics(); 

		// update game
		if(g_pApp->mpGame) {
			// clear events
			mfFreeSpaceEventStack = g_pEventStack->getFreeSpace();
			safeTickEventManager();
			g_pEventStack->swapBuffers();
			g_pEventStack->clearCurrentBuffer();

			// input
			mpInput->capture();

			// inject elapsed time to CEGUI
			CEGUI::System::getSingleton().injectTimePulse(g_fElapsedTime);


			// update game process manager/game views/game state
			g_pApp->mpGame->VOnUpdate();
		}

		// swap buffers
		mpWindow->swapBuffers(true);

		Ogre::WindowEventUtilities::messagePump();

		g_pClock->Update();
	}

	OnClose();
}



// =====================================================================================================================================================
// =============================================== BaseGameLogic =======================================================================================
// =====================================================================================================================================================

BaseGameLogic::BaseGameLogic() {
	mpProcessManager = K_new CProcessManager();
	mRandom.Randomize();
	mbRenderDiagnostics = false;
	mGameViews.clear();

	mState = BGS_Initializing;
	mbPaused = false;
}

BaseGameLogic::~BaseGameLogic() {
	VDestroy();
}

void BaseGameLogic::VClear() {
	mbRenderDiagnostics = false;
	mbPaused = false;
	mExpectedPlayers = 0;
}

void BaseGameLogic::VDestroy() {
	gActorMgr->RemoveAllActors();
	VRemoveAllViews();

	SAFE_DELETE(mpProcessManager);

	VClear();
}

//				VGetRandomActor
/*ActorId BaseGameLogic::VGetRandomActor(ActorId ignore) {
	int max = mActorMap.size();
	if(max == 0)
		return 0;

	int c = mRandom.Random(max);

	ActorMap::iterator it = mActorMap.begin();
	for(int i=0; i<c; i++)
		it++;

	if(it->first == ignore) {
		it++;

		if(it == mActorMap.end())
			it = mActorMap.begin();
	}

	return it->first;
}*/

//				VAddActor
HActor BaseGameLogic::VAddActor(char *chName, int type) {
	HActor handle = gActorMgr->CreateActor(chName, type);
	if(handle.IsNull()) {
		ERR("VAddActor failed");
		g_pApp->AbortGame();
	}
	
	return handle;
}

//				VRemoveActor
void BaseGameLogic::VRemoveActor(HActor &handle) {
	gActorMgr->RemoveActor(handle);
}

//				VGetActor
IActor* const BaseGameLogic::VGetActor(HActor &handle) {
	IActor *pAc = gActorMgr->GetActor(handle);
	assert(pAc && "VGetActor: Unknown actor");

	return pAc;
}

//				VOnUpdate
void BaseGameLogic::VOnUpdate() {
	int deltaMilli = int(g_fElapsedTime * 1000.0f);
	EvtData_GameState *evt = NULL;

	switch(mState) {
		case BGS_Initializing:
			evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
			evt->EvtData_GameState::EvtData_GameState(BGS_MainMenu);
			safeTriggerEvent(evt);
			break;

		case BGS_MainMenu:
			//VChangeState(BGS_LoadingEnvironment);
			break;

		case BGS_SpawnAI:
			// nothing
			break;

		case BGS_LoadingEnvironment:
			if(VLoadGame("NewGame")) {
				evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
				evt->EvtData_GameState::EvtData_GameState(BGS_Running);
				safeTriggerEvent(evt);
			} else {
				assert(0 && "Failed to Load Game");
			}
			break;

		case BGS_Running:
			mpProcessManager->UpdateProcesses(deltaMilli);
			break;
	}

	// Update GameViews
	GameViewList::iterator begin = mGameViews.begin();
	GameViewList::iterator end = mGameViews.end();

	while(begin != end) {
		(*begin)->VOnUpdate(deltaMilli);
		begin++;
	}
}

//				VChangeState
void BaseGameLogic::VChangeState(BaseGameState newstate) {
	mState = newstate;

	// Send an Event with the new state
	/*EvtData_GameState *evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
	evt->EvtData_GameState::EvtData_GameState(newstate);
	safeQueEvent(evt); */
}

//				VRenderDiagnostics
void BaseGameLogic::VRenderDiagnostics() {
	if(mbRenderDiagnostics) {
		// do something
	}
}

//				TogglePause
void BaseGameLogic::TogglePause() {
	mbPaused = !mbPaused;

	if(mbPaused) {
		g_pClock->SetPaused(true);
	} else {
		g_pClock->SetPaused(false);
	};
}

//				VAddView
void BaseGameLogic::VAddView(IGameView *pView, ActorId aid) {
	int viewId = mGameViews.size();
	mGameViews.push_back(pView);
	pView->VOnAttach(viewId, aid);
}

void BaseGameLogic::VRemoveView(IGameView *pView) {
	if(pView) {
		mGameViews.remove(pView);
		SAFE_DELETE(pView);
	}
}

void BaseGameLogic::VRemoveAllViews() {
	while(mGameViews.size() != 0) {
		VRemoveView(mGameViews.front());
	}
}


// =====================================================================================================================================================
// =============================================== HumanView ===========================================================================================
// =====================================================================================================================================================

const unsigned int SCREEN_REFRESH_RATE(1000/60);

//				HumanView
HumanView::HumanView() {
	mId = -1;
	mAid = -1;
	mpProcessManager = K_new CProcessManager();
}

//				~HumanView
HumanView::~HumanView() {

	SAFE_DELETE(mpProcessManager);
}


//				VOnUpdate
void HumanView::VOnUpdate(int deltaMilli) {
	mpProcessManager->UpdateProcesses(deltaMilli);
}

