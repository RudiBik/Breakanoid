#include "KurveStd.h"

#include "..\GameApp.h"
#include "KurveView.h"

#include "..\EventManager\StackAllocator.h"
#include "KurveApp.h"

#include "KurveGame.h"
#include "KurveActors.h"
#include "KurveProcesses.h"

#include <OGRE/OgreMeshManager.h>

// MAINMENU IDS
#define MM_BACKGROUND (2)
#define MM_PLAY		  (3)
#define MM_OPTIONS		(4)
#define MM_EXIT			(5)


// GameViewEventListener
class GameViewEventListener : public IEventListener {
	GameView *mpGameView;

public:
	explicit GameViewEventListener(GameView *pView) {
		mpGameView = pView;
	}

	virtual ~GameViewEventListener() {}
	virtual char* GetName() { return "GameViewEventListener"; }

	virtual bool HandleEvent(IEventData *pEvt);
};

bool GameViewEventListener::HandleEvent(IEventData *pEvt) {
	if(pEvt->VGetEventType() == EvtData_GameState::sEventType) {
		EvtData_GameState *pGameState = static_cast<EvtData_GameState*>(pEvt);

		mpGameView->HandleGameState(pGameState->mState);

	} else if(EvtData_Collision::sEventType == pEvt->VGetEventType()) {
		EvtData_Collision *castEvent = static_cast<EvtData_Collision*>(pEvt);


		if(castEvent->type == CT_Paddle_Ball) 
			g_pApp->mpDevice->PlaySound(g_pApp->mSoundPaddle, false);
		else if(castEvent->type == CT_Ball_Box) {
			
			UINT x = g_pApp->mpGame->mRandom.Random(3);
			g_pApp->mpDevice->PlaySound(g_pApp->mSoundBlock[x], false);
		} else if(castEvent->type == CT_Ball_Wall) {
			g_pApp->mpDevice->PlaySound(g_pApp->mSoundWall, false);
		}

	// Move Actor
	} else if(EvtData_Move_Actor::sEventType == pEvt->VGetEventType()) {
		EvtData_Move_Actor *castEvent = static_cast<EvtData_Move_Actor*>(pEvt);

		Ogre::SceneNode *pNode = mpGameView->GetSceneNode(castEvent->mHandle);
		if(!pNode) {
			//assert(0 && "KurveView: Listener->EvtData_Move_Actor: pNode == NULL");
			return false;
		}

		pNode->setPosition(castEvent->mPos);
		pNode->setOrientation(castEvent->mOrient);
		pNode->setScale(castEvent->mScale);


		return false;

	// New Actor
	} else if(pEvt->VGetEventType() == EvtData_New_Actor::sEventType) {
		EvtData_New_Actor *pCastEvent = static_cast<EvtData_New_Actor*>(pEvt);

		IActor *pAc = gActorMgr->GetActor(pCastEvent->mHandle);
		Ogre::SceneNode *pNode = gActorMgr->GetSceneNode(pCastEvent->mHandle, mpGameView->mpScene);
		mpGameView->AddSceneNode(pCastEvent->mHandle, pNode);

		if(pAc->VGetType() == AT_Paddle)
			mpGameView->mpPaddle = pNode;
		else if(pAc->VGetType() == AT_Ball)
			mpGameView->mpBall = pNode;
	
	// destroy Actor
	} else if(pEvt->VGetEventType() == EvtData_Destroy_Actor::sEventType) {
		EvtData_Destroy_Actor *pCastEvent = static_cast<EvtData_Destroy_Actor*>(pEvt);
		
		Ogre::SceneNode *pNode = mpGameView->GetSceneNode(pCastEvent->mHandle);
		if(pNode == mpGameView->mpPaddle)
			mpGameView->mpPaddle = NULL;
		else if(pNode == mpGameView->mpBall)
			mpGameView->mpBall = NULL;

		mpGameView->mpScene->destroySceneNode(pNode);
		mpGameView->RemoveSceneNode(pCastEvent->mHandle);
	} 

	return false;
}


// GameView
GameView::GameView() {
	HumanView::HumanView();
	mState = BGS_LoadingEnvironment;

	// Buttons
	mbPDown = false;
	mbESCDown = false;

	mbWon = false;
	mbMenu = false;
	mpPaddle = NULL;
	mpBall = NULL;

	// DefaultManager
	Ogre::String sSceneName = "DefaultSceneManager";
	mpScene = g_pApp->mpRoot->createSceneManager(sSceneName);

	// Camera
	mpCamera = mpScene->createCamera("DefaultCamera");
	mpCameraNode = mpScene->getRootSceneNode()->createChildSceneNode();
	mpCameraNode->attachObject(mpCamera);
	mpCameraNode->setPosition(0, 10, 19);
	mpCameraNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_WORLD); 

	mpCamera->setNearClipDistance(1.5f);

	// VP
	mpVP = g_pApp->mpWindow->addViewport(mpCamera);
	mpVP->setBackgroundColour(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
	mpVP->setAutoUpdated(true);

	mpCamera->setAspectRatio(Ogre::Real(mpVP->getActualWidth()) / Ogre::Real(mpVP->getActualHeight()));

	mpScene->setAmbientLight(Ogre::ColourValue(0.1f, 0.1f, 0.1f));
	mpScene->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE); 


	// CEGUI Window
	CEGUI::WindowManager &wMgr = CEGUI::WindowManager::getSingleton();
	mSheet = wMgr.loadWindowLayout("Paused.layout");
	CEGUI::Window *contBut = wMgr.getWindow("PRoot/Frame/Con");
	contBut->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameView::ContinueButton, this));
	CEGUI::Window *backBut = wMgr.getWindow("PRoot/Frame/Ret");
	backBut->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameView::BackButton, this));

	CEGUI::System::getSingleton().setGUISheet(mSheet);
	mSheet->setVisible(false);
	CEGUI::MouseCursor::getSingleton().setVisible(false);

	// create the listener
	mpListener = shared_ptr<IEventListener>(K_new GameViewEventListener(this));

	// register events
	safeAddListener(mpListener, EvtData_GameState::sEventType);
	safeAddListener(mpListener, EvtData_Collision::sEventType);
	safeAddListener(mpListener, EvtData_Fire::sEventType);
	safeAddListener(mpListener, EvtData_New_Actor::sEventType);
	safeAddListener(mpListener, EvtData_Move_Actor::sEventType);
	safeAddListener(mpListener, EvtData_Destroy_Actor::sEventType);

	g_pApp->mpDevice->PlaySound(g_pApp->mSoundBack, true);
}

GameView::~GameView() {
	g_pApp->mpDevice->StopSound(g_pApp->mSoundBack);

	safeDelListener(mpListener, EvtData_GameState::sEventType);
	safeDelListener(mpListener, EvtData_Collision::sEventType);
	safeDelListener(mpListener, EvtData_Fire::sEventType);
	safeDelListener(mpListener, EvtData_New_Actor::sEventType);
	safeDelListener(mpListener, EvtData_Move_Actor::sEventType);
	safeDelListener(mpListener, EvtData_Destroy_Actor::sEventType);

	CEGUI::WindowManager::getSingleton().destroyAllWindows();

	mSceneNodeMap.clear();
	g_pApp->mpRoot->destroySceneManager(mpScene);
	g_pApp->mpWindow->removeViewport(mpVP->getZOrder());
}

bool GameView::updateCEGUI(const CEGUI::EventArgs& e) {
	return false;
}

bool GameView::ContinueButton(const CEGUI::EventArgs& e) {
	EvtData_GameState *pEvt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
	pEvt->EvtData_GameState::EvtData_GameState(BGS_Running);
	safeQueEvent(pEvt);
	mbMenu = !mbMenu;

	return true;
}

bool GameView::BackButton(const CEGUI::EventArgs& e) {
	EvtData_GameState *pEvt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
	pEvt->EvtData_GameState::EvtData_GameState(BGS_MainMenu);
	safeQueEvent(pEvt);

	KurveGame *pGame = static_cast<KurveGame*>(g_pApp->mpGame);
	pGame->SaveScore();

	return true;
}

// Update
void GameView::VOnUpdate(int delta) {
	HumanView::VOnUpdate(delta);
	float fTimeScale;

	// Input
	if(g_pApp->mpInput) {
		MovementController *pController = g_pApp->mpInput;

		// steer left
		if(mbWon) {
			if(pController->getKeyboard()->isKeyDown(OIS::KC_RETURN)) {
				EvtData_GameState *evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
				evt->EvtData_GameState::EvtData_GameState(BGS_MainMenu);
				safeQueEvent(evt);

				TextRenderer::getSingleton().removeTextBox("Won");
				mbWon = false;
				return;
			} else
				return;
		}
		if(pController->getKeyboard()->isKeyDown(OIS::KC_LEFT)) {
			EvtData_Steer *pEvt = static_cast<EvtData_Steer*>(g_pEventStack->alloc(sizeof(EvtData_Steer), ALIGNOF(EvtData_Steer)));
			pEvt->EvtData_Steer::EvtData_Steer(-1.0f);
			safeQueEvent(pEvt);
		} 
		// steer right
		if(pController->getKeyboard()->isKeyDown(OIS::KC_RIGHT)) {
			EvtData_Steer *pEvt = static_cast<EvtData_Steer*>(g_pEventStack->alloc(sizeof(EvtData_Steer), ALIGNOF(EvtData_Steer)));
			pEvt->EvtData_Steer::EvtData_Steer(1.0f);
			safeQueEvent(pEvt);
		} 
		// fire
		if(pController->getKeyboard()->isKeyDown(OIS::KC_SPACE)) {
			EvtData_Fire *pEvt = static_cast<EvtData_Fire*>(g_pEventStack->alloc(sizeof(EvtData_Fire), ALIGNOF(EvtData_Fire)));
			pEvt->EvtData_Fire::EvtData_Fire();
			safeQueEvent(pEvt);
		}

		if(pController->getKeyboard()->isKeyDown(OIS::KC_ADD)) {
			fTimeScale = g_pClock->GetTimeScale();
			if(fTimeScale < 2.0f) 
				fTimeScale += 0.05f;
			g_pClock->SetTimeScale(fTimeScale);
		}

		if(pController->getKeyboard()->isKeyDown(OIS::KC_SUBTRACT)) {
			fTimeScale = g_pClock->GetTimeScale();
			if(fTimeScale > 0.2f) 
				fTimeScale -= 0.05f;
			g_pClock->SetTimeScale(fTimeScale);
		}

		if(pController->getKeyboard()->isKeyDown(OIS::KC_ESCAPE) && !mbESCDown) {
			mbMenu = !mbMenu;
			mbESCDown = true;

			if(mbMenu) {
				EvtData_GameState *pEvt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
				pEvt->EvtData_GameState::EvtData_GameState(BGS_Menu);
				safeQueEvent(pEvt);
			} else {
				EvtData_GameState *pEvt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
				pEvt->EvtData_GameState::EvtData_GameState(BGS_Running);
				safeQueEvent(pEvt);
			}
		} else if(!pController->getKeyboard()->isKeyDown(OIS::KC_ESCAPE)) 
			mbESCDown = false;

		if(!pController->getKeyboard()->isKeyDown(OIS::KC_P) && mbPDown) {
			g_pApp->mpGame->TogglePause();

			if(g_pApp->mpGame->IsPaused()) 
				TextRenderer::getSingleton().addTextBox("Paused", "PAUSED", 0, 0, 200, 64, FFLAG_HCENTER | FFLAG_VCENTER, "Arial-64", "64",
				Ogre::ColourValue(0.0f, 0.0f, 0.0f));
			else
				TextRenderer::getSingleton().removeTextBox("Paused");
	
			mbPDown = false;
		} else if(pController->getKeyboard()->isKeyDown(OIS::KC_P)) 
			mbPDown = true;
	}
}


// VBuildInitialScene
void GameView::VBuildInitialScene() {
	// add base level
	Ogre::Entity *pEnt = mpScene->createEntity("level.mesh");
	pEnt->setCastShadows(false);
	Ogre::SceneNode *pNode = mpScene->getRootSceneNode()->createChildSceneNode();
	pNode->attachObject(pEnt);
	pNode->translate(0.0f, 0.0f, 0.0f);

	// add directional light
/*	Ogre::Light *pLight = mpScene->createLight("PointLight1");
	pLight->setType(Ogre::Light::LT_POINT);
	pLight->setPosition(0.0f, 8.0f, 8.0f);
	pLight->setDiffuseColour(1.0f, 1.0f, 1.0f);
	pLight->setSpecularColour(1.0f, 1.0f, 1.0f);
	Ogre::SceneNode *pLightNode = mpScene->getRootSceneNode()->createChildSceneNode();
	pLightNode->attachObject(pLight);  */
	 
	// add directional light
	Ogre::Light *pLight = mpScene->createLight("PointLight1");
	pLight->setType(Ogre::Light::LT_POINT);
	pLight->setPosition(Ogre::Vector3(-2.0f, 5.0f, 2.0f)); 
	pLight->setDiffuseColour(1.0f, 1.0f, 1.0f);
	pLight->setSpecularColour(1.0f, 1.0f, 1.0f);
	Ogre::SceneNode *pLightNode = mpScene->getRootSceneNode()->createChildSceneNode();
	pLightNode->attachObject(pLight); 

	// add skybox
	mpScene->setSkyBox(true, "Examples/SpaceSkyBox");
} 

void GameView::AddSceneNode(UINT handle, Ogre::SceneNode *pNode) {
	mSceneNodeMap[handle] = pNode;
}

Ogre::SceneNode* GameView::GetSceneNode(UINT handle) {
	SceneNodeMap::iterator it = mSceneNodeMap.find(handle);

	if(it == mSceneNodeMap.end()) {
		return NULL;
	} else {
		return it->second;
	}
}

void GameView::RemoveSceneNode(UINT handle) {
	Ogre::SceneNode *pNode = GetSceneNode(handle);
	if(pNode != NULL) {
		mSceneNodeMap.erase(handle);

		mpScene->getRootSceneNode()->removeChild(pNode);
	} else {
		//assert(0 && "RemoveSceneNode: Unknown id");
	}
}

// HandleGameState
void GameView::HandleGameState(BaseGameState state) {
	mState = state;

	KurveGame *pGame = static_cast<KurveGame*>(g_pApp->mpGame);

	if(state == BGS_Won) {
		g_pApp->mpDevice->PlaySound(g_pApp->mSoundWin, false);
		TextRenderer::getSingleton().addTextBox("Won", "", 0, 0, 200, 64, FFLAG_HCENTER, "Arial-64", "32"); 
		TextRenderer::getSingleton().printf("Won", "You Win \n    Your Score: %d \n Press Enter", pGame->GetScore());
		mbWon = true;
	} else if(state == BGS_Lose) {
		TextRenderer::getSingleton().addTextBox("Won", "", 0, 0, 200, 64, FFLAG_HCENTER, "Arial-64", "32"); 
		TextRenderer::getSingleton().printf("Won", "You Lose \n Your Score: %d \n Press Enter", pGame->GetScore());
		mbWon = true;
	} else if(state == BGS_Menu) {
		//g_pDevice->StopAll();
		CEGUI::MouseCursor::getSingleton().setVisible(true);
		mSheet->setVisible(true);
	} else if(state == BGS_Running) {
		//g_pDevice->PlaySound(mSoundBack, true);
		mSheet->setVisible(false);
		CEGUI::MouseCursor::getSingleton().setVisible(false);
	}
}



// MainMenuView

bool MainMenuView::HighscoreLayout(const CEGUI::EventArgs &args) {
	CEGUI::WindowManager &wMgr = CEGUI::WindowManager::getSingleton();
	KurveGame *pGame = static_cast<KurveGame*>(g_pApp->mpGame);
	UINT *points = pGame->GetHighScore();

	CEGUI::Window *p = wMgr.getWindow("highscoreRoot/frame/1");
	sprintf(buffer, "%d", points[0]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/2");
	sprintf(buffer, "%d", points[1]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/3");
	sprintf(buffer, "%d", points[2]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/4");
	sprintf(buffer, "%d", points[3]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/5");
	sprintf(buffer, "%d", points[4]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/6");
	sprintf(buffer, "%d", points[5]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/7");
	sprintf(buffer, "%d", points[6]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/8");
	sprintf(buffer, "%d", points[7]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/9");
	sprintf(buffer, "%d", points[8]);
	p->setText(buffer);

	p = wMgr.getWindow("highscoreRoot/frame/10");
	sprintf(buffer, "%d", points[9]);
	p->setText(buffer);

	return true;
}