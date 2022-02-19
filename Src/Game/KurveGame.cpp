#include "KurveStd.h"
#include "KurveGame.h"

#include "..\ResourceCache\ResCache.h"
#include "..\EventManager\Events.h"
#include "..\EventManager\EventManagerImpl.h"
#include "KurveView.h"
#include "KurveProcesses.h"
#include <algorithm>

// todo ondestroy effects


//========================================================================
//
// Game Event Listener
//
//========================================================================

class KurveEventListener : public IEventListener {
public:
	explicit KurveEventListener(KurveGame *game) { mpGame = game; }
	virtual ~KurveEventListener() {}
	virtual char* GetName() { return "KurveEventListener"; }

	bool HandleEvent(IEventData *evt);

private:
	KurveGame *mpGame;
};

bool KurveEventListener::HandleEvent(IEventData *evt) {
	if(EvtData_Steer::sEventType == evt->VGetEventType()) {
		EvtData_Steer *castEvent = static_cast<EvtData_Steer*>(evt);

		if(!mpGame->mhPaddle.IsNull())
			mpGame->VGetActor(mpGame->mhPaddle)->VAddVelocity(Ogre::Vector3(50.0f * castEvent->mfSteer * g_fElapsedTime, 0.0f, 0.0f));


	// Collision
	} else if(EvtData_Collision::sEventType == evt->VGetEventType()) {
		EvtData_Collision *castEvent = static_cast<EvtData_Collision*>(evt);

		IActor* pActorA = mpGame->VGetActor(castEvent->mHandle1);
		IActor* pActorB = mpGame->VGetActor(castEvent->mHandle2);

		if(pActorA == NULL || pActorB == NULL || mpGame->mhPaddle.IsNull()) {
			assert(0 && "collisionEvt: IActor == NULL");
			return false;
		}

		if(castEvent->type == CT_Ball_Box) {
			// decrease the life of the block
			Block *pBlock = static_cast<Block*>(pActorB);
			pBlock->mLifes--;

			// increase score
			mpGame->mScore += static_cast<UINT>(200 * mpGame->mfBlockScale * mpGame->mfScoreMultiplicator);
			if(pBlock->mLifes <= 0) {
				mpGame->mScore += static_cast<UINT>(400 * mpGame->mfBlockScale * mpGame->mfScoreMultiplicator);

				// send destroy event
				EvtData_Destroy_Actor *pDestroyEvent = static_cast<EvtData_Destroy_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Destroy_Actor), ALIGNOF(EvtData_Destroy_Actor)));
				pDestroyEvent->EvtData_Destroy_Actor::EvtData_Destroy_Actor(pBlock->mHandle);
				safeTriggerEvent(pDestroyEvent);

				// ondestroy effects
				if(pBlock->mBE == Block::BE_Paddle) { // increased Paddle
					shared_ptr<CPaddleEffectProcess> pEffect(K_new CPaddleEffectProcess(15.0f, mpGame->mhPaddle));
					mpGame->mpProcessManager->Attach(pEffect);
				} else if(pBlock->mBE == Block::BE_Points) {
					shared_ptr<CPointsProcess> pPointsEffect(K_new CPointsProcess(20.0f, &mpGame->mfScoreMultiplicator));
					mpGame->mpProcessManager->Attach(pPointsEffect);

					shared_ptr<CTextProcess> pTextProcess(K_new CTextProcess(20.0f, "PointsEffectText", "DOUBLE POINTS", -200, -300, 100, 100, FFLAG_VCENTER | FFLAG_HCENTER, "Arial-64", "64", Ogre::ColourValue(0.0f, 0.8f, 0.5f, 1.0f)));
					mpGame->mpProcessManager->Attach(pTextProcess);
				}
			}

			// increase point multiplicator
			mpGame->mfBlockScale += 0.1f;
		} else if(castEvent->type == CT_Paddle_Ball) {
			mpGame->mfBlockScale = 1.0f;
		}

	// Request start game
	} else if(EvtData_Request_Start_Game::sEventType == evt->VGetEventType()) {
		// Remove menu view
		EvtData_Request_Start_Game *pStartGame = static_cast<EvtData_Request_Start_Game*>(evt);
		mpGame->VRemoveView(pStartGame->mpView);

		// add gameview
		IGameView *pGameView = K_new GameView();
		mpGame->VAddView(pGameView);

		EvtData_New_Game *pStartGameEvt = static_cast<EvtData_New_Game*>(g_pEventStack->alloc(sizeof(EvtData_New_Game), ALIGNOF(EvtData_New_Game)));
		pStartGameEvt->EvtData_New_Game::EvtData_New_Game();
		safeQueEvent(pStartGameEvt);

		EvtData_GameState *evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
		evt->EvtData_GameState::EvtData_GameState(BGS_LoadingEnvironment);
		safeQueEvent(evt);

	// Fire Ball
	} else if(EvtData_Fire::sEventType == evt->VGetEventType()) {
		EvtData_Fire *castEvent = static_cast<EvtData_Fire*>(evt);

		if(!mpGame->mhPaddle.IsNull() && !mpGame->mhBall.IsNull()) {
			Paddle *pPaddle = static_cast<Paddle*>(mpGame->VGetActor(mpGame->mhPaddle));
			pPaddle->Fire();
		}

	// New Game
	} else if(EvtData_New_Game::sEventType == evt->VGetEventType()) {
		// create start scene

	
	// New Actor
	} else if(EvtData_New_Actor::sEventType == evt->VGetEventType()) {
		EvtData_New_Actor *castEvent = static_cast<EvtData_New_Actor*>(evt);

		if(castEvent->mHandle.IsNull()) {
			assert(0 && "New Actor Event: mHandle == NULL");
			return false;
		}

		IActor *pAct = gActorMgr->GetActor(castEvent->mHandle);
		if(!pAct)
			return false;

		int type = pAct->VGetType();
		if(type == AT_Block) {
			mpGame->mBlockList.push_back(castEvent->mHandle);
		} else if(type == AT_Paddle)
			mpGame->mhPaddle = castEvent->mHandle;
		else if(type == AT_Ball) {
			if(!mpGame->mhPaddle.IsNull()) {
				Paddle *pPaddle = static_cast<Paddle*>(gActorMgr->GetActor(mpGame->mhPaddle));
				pPaddle->SetBall(castEvent->mHandle);
				mpGame->mhBall = castEvent->mHandle;
			}
		}


	// missed ball
	} else if(EvtData_MissedBall::sEventType == evt->VGetEventType()) {
		EvtData_MissedBall *castEvent = static_cast<EvtData_MissedBall*>(evt);

		// actor already destroyed
		if(mpGame->mTriesLeft > 0) {
			if(!mpGame->mhPaddle.IsNull()) {
				HActor hBall = mpGame->VAddActor("Ball", AT_Ball);
				Ball *pBall = static_cast<Ball*>(mpGame->VGetActor(hBall));
				pBall->mbGrabbed = true;
				pBall->VSetPosition(Ogre::Vector3(0.0f, -0.2f, -0.25f));
				pBall->mhPaddle = mpGame->mhPaddle;

				EvtData_New_Actor *pBallEvt = static_cast<EvtData_New_Actor*>(g_pEventStack->alloc(sizeof(EvtData_New_Actor), ALIGNOF(EvtData_New_Actor)));
				pBallEvt->EvtData_New_Actor::EvtData_New_Actor(hBall);
				safeQueEvent(pBallEvt);
			}

			mpGame->mTriesLeft--;
		} else {
			// Game Over
			mpGame->SaveScore();

			if(mpGame->mTriesLeft < 0)
				mpGame->mTriesLeft = 0;

			EvtData_GameState *pGameOver = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
			pGameOver->EvtData_GameState::EvtData_GameState(BGS_Lose);
			safeQueEvent(pGameOver);		
		}


	// Destroy Actor
	} else if(EvtData_Destroy_Actor::sEventType == evt->VGetEventType()) {
		EvtData_Destroy_Actor *castEvent = static_cast<EvtData_Destroy_Actor*>(evt);

		IActor *pActor = mpGame->VGetActor(castEvent->mHandle);
		if(pActor == NULL)
			return false;

		int aType = pActor->VGetType();
		if(aType == AT_Block) {
			mpGame->mBlockList.remove(castEvent->mHandle);
		} else if(aType == AT_Ball) {
			mpGame->mhBall = HActor();

		} else if(aType == AT_Paddle) {
			mpGame->mhPaddle = HActor();
		}

		mpGame->VRemoveActor(castEvent->mHandle);


	// Move Actor
	/*} else if(EvtData_Move_Actor::sEventType == evt->VGetEventType()) {
		EvtData_Move_Actor *castEvent = static_cast<EvtData_Move_Actor*>(evt);

		shared_ptr<IActor> pActor = mpGame->VGetActor(castEvent->mId);
		pActor->VSetPosition(&castEvent->mPos);
		pActor->VSetOrientation(&castEvent->mOrient);
		pActor->VSetScale(&castEvent->mScale);

		return true; */
	} else if(EvtData_GameState::sEventType == evt->VGetEventType()) {
		EvtData_GameState *castEvent = static_cast<EvtData_GameState*>(evt);

		mpGame->VChangeState(castEvent->mState);

		return false;
	}

	return false;
}


//========================================================================
//
// Game Logic
//
//========================================================================

KurveGame::KurveGame() : BaseGameLogic(),
	mfLifeTime(0), mScore(0), mTriesLeft(5), mLevel(0), mbInitializing(false), mfBlockScale(1.0f) {

	// Register Game Events
	g_pApp->mpEventMgr->RegisterEvent(EvtData_Fire::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_New_Actor::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_Collision::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_Move_Actor::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_Steer::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_Destroy_Actor::sEventType);
	g_pApp->mpEventMgr->RegisterEvent(EvtData_MissedBall::sEventType);

	VInitialize();
}

KurveGame::~KurveGame() {}

bool KurveGame::VLoadGame(std::string name) {
	// User clicked on Start
	if(name == "NewGame") {
		VBuildInitialScene();
	}

	return true;
}

void KurveGame::VOnUpdate() {
	int deltaMilli = int(g_fElapsedTime * 1000.0f);
	mfLifeTime += g_fElapsedTime;

	BaseGameLogic::VOnUpdate();

	switch(mState) {
		case BGS_Running: 
		{
			// Level/Game ended
			if(mBlockList.size() == 0 && !mbInitializing) {
				mScore += mLevel * 2500 * mfScoreMultiplicator;

				mLevel++;
				if(mLevel > 8) { // Game Ended
					EvtData_GameState *evt = static_cast<EvtData_GameState*>(g_pEventStack->alloc(sizeof(EvtData_GameState), ALIGNOF(EvtData_GameState)));
					evt->EvtData_GameState::EvtData_GameState(BGS_Won);
					safeQueEvent(evt);
					return;
				}

				LoadLevel(mLevel);
				break;
			} else if(mbInitializing) 
				mbInitializing = !mbInitializing;

			RenderText();

			// update Camera
			GameView *pView = static_cast<GameView*>(mGameViews.front());
			if(!mhPaddle.IsNull())
				pView->getCamNode()->setPosition(VGetActor(mhPaddle)->VGetPosition() + Ogre::Vector3(0.0f, 10.0f, 10.0f));

			if(!mhBall.IsNull()) 
				pView->getCamNode()->lookAt(VGetActor(mhBall)->VGetPosition() * 0.5f, Ogre::SceneNode::TS_WORLD);
			else 
				pView->getCamNode()->lookAt(Ogre::Vector3::ZERO, Ogre::SceneNode::TS_WORLD);


			// Update Objects
			if(!mhPaddle.IsNull())
				VGetActor(mhPaddle)->VOnUpdate(deltaMilli);
			if(!mhBall.IsNull())
				VGetActor(mhBall)->VOnUpdate(deltaMilli);

			// check for collisions
			blockList::iterator itBegin = mBlockList.begin();
			blockList::iterator itEnd = mBlockList.end();
			while(itBegin != itEnd) {

				CheckForCollisions(itBegin);

				// increase iterator at the end
				itBegin++;
			} // while

		} break;

		case BGS_Won:
		{
			RenderText();
			// todo
		} break;

		case BGS_Lose: 
		{
			RenderText();
			// todo
		} break;

		case BGS_Menu: 
		{
			// todo
		} break;
	}
}

void KurveGame::VClear() {
	BaseGameLogic::VClear();

	mfLifeTime  = 0.0f;
	mTriesLeft = 7;
	mLevel = 1;
	mScore = 0;
	mbInitializing = true;
	mbPaused = false;
	mfBlockScale = 1.0f;
	mfScoreMultiplicator = 1.0f;

	// Open the highscorefile
	FILE *pFile = fopen("highscore.dat", "r+");
	if(pFile == NULL) {
		assert(0 && "highscore.dat not found!");
		g_pApp->AbortGame();
	}

	for(int i=0; i<10; i++) {
		fread(&mPoints[i], sizeof(UINT), 1, pFile);
	}
	fclose(pFile);

	mhPaddle = HActor();
	mhBall = HActor();
	mBlockList.clear();

	VRemoveAllViews();
	gActorMgr->RemoveAllActors();
}

void KurveGame::VDestroy() {
	BaseGameLogic::VDestroy();

	safeDelListener(mpKurveListener, EvtData_GameState::sEventType);
	safeDelListener(mpKurveListener, EvtData_Request_Start_Game::sEventType);
	safeDelListener(mpKurveListener, EvtData_New_Game::sEventType);
	safeDelListener(mpKurveListener, EvtData_New_Actor::sEventType);
	safeDelListener(mpKurveListener, EvtData_Fire::sEventType);
	safeDelListener(mpKurveListener, EvtData_Collision::sEventType);
	safeDelListener(mpKurveListener, EvtData_Move_Actor::sEventType);
	safeDelListener(mpKurveListener, EvtData_Steer::sEventType);
	safeDelListener(mpKurveListener, EvtData_Destroy_Actor::sEventType);
	safeDelListener(mpKurveListener, EvtData_MissedBall::sEventType);

	TextRenderer::getSingleton().removeTextBox("Lifes");
	// Level
	TextRenderer::getSingleton().removeTextBox("Level");
	// Fps
	TextRenderer::getSingleton().removeTextBox("FPS");
	// Stack
	TextRenderer::getSingleton().removeTextBox("Stack");
	// TimeScale
	TextRenderer::getSingleton().removeTextBox("TimeScale");
	// Score
	TextRenderer::getSingleton().removeTextBox("Score");
	// BlockScale
	TextRenderer::getSingleton().removeTextBox("BlockScale");
	TextRenderer::getSingleton().removeTextBox("Milliseconds");
}

void KurveGame::VInitialize() {
	BaseGameLogic();

	VClear();

	mpKurveListener = shared_ptr<KurveEventListener>(K_new KurveEventListener(this));

	safeAddListener(mpKurveListener, EvtData_GameState::sEventType);
	safeAddListener(mpKurveListener, EvtData_Request_Start_Game::sEventType);
	safeAddListener(mpKurveListener, EvtData_New_Game::sEventType);
	safeAddListener(mpKurveListener, EvtData_New_Actor::sEventType);
	safeAddListener(mpKurveListener, EvtData_Fire::sEventType);
	safeAddListener(mpKurveListener, EvtData_Collision::sEventType);
	safeAddListener(mpKurveListener, EvtData_Move_Actor::sEventType);
	safeAddListener(mpKurveListener, EvtData_Steer::sEventType);
	safeAddListener(mpKurveListener, EvtData_Destroy_Actor::sEventType);
	safeAddListener(mpKurveListener, EvtData_MissedBall::sEventType);

	// TextBoxes
	// Lifes
	printf(buffer, "Lifes: %d", mTriesLeft);
	TextRenderer::getSingleton().addTextBox("Lifes", buffer, 10, 10, 100, 100);
	// Level
	TextRenderer::getSingleton().addTextBox("Level", "Level: x", 10, 30, 100, 100);
	// Fps
	TextRenderer::getSingleton().addTextBox("FPS", "FPS: x", 10, 50, 100, 100);
	// Stack
	TextRenderer::getSingleton().addTextBox("Stack", "Stack: x", 10, 70, 100, 100);
	// TimeScale
	TextRenderer::getSingleton().addTextBox("TimeScale", "TimeScale: x", 10, 90, 100, 100);
	// Score
	TextRenderer::getSingleton().addTextBox("Score", "Score: x", 10, 110, 100, 100);
	// BlockScale
	TextRenderer::getSingleton().addTextBox("BlockScale", "BlockScale: x", 10, 130, 100, 100);
	// milliseconds
	TextRenderer::getSingleton().addTextBox("Milliseconds", "Milliseconds: x", 10, 150, 100, 100);
}

void KurveGame::VBuildInitialScene() {
	// create paddle
	HActor hPaddle = VAddActor("Paddle", AT_Paddle);
	Paddle *pPaddle = static_cast<Paddle*>(VGetActor(hPaddle));
	pPaddle->mbBall = false;
	pPaddle->VSetPosition(Ogre::Vector3(0.0f, -0.15f, 8.5f));

	EvtData_New_Actor *pRequestActor = static_cast<EvtData_New_Actor*>(g_pEventStack->alloc(sizeof(EvtData_New_Actor), ALIGNOF(EvtData_New_Actor)));
	pRequestActor->EvtData_New_Actor::EvtData_New_Actor(hPaddle);
	safeTriggerEvent(pRequestActor); 

	// create ball
	HActor hBall = VAddActor("Ball", AT_Ball);
	Ball *pBall = static_cast<Ball*>(VGetActor(hBall));
	pBall->mbGrabbed = true;
	pBall->mhPaddle = hPaddle;
	pBall->VSetPosition(Ogre::Vector3(0.0f, -0.20f, -0.25f));

	pRequestActor = static_cast<EvtData_New_Actor*>(g_pEventStack->alloc(sizeof(EvtData_New_Actor), ALIGNOF(EvtData_New_Actor)));
	pRequestActor->EvtData_New_Actor::EvtData_New_Actor(hBall);
	safeQueEvent(pRequestActor); 

	CreateBlockRow("12  4  21", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
	CreateBlockRow("         ", Ogre::Vector3(-8.0f, 0.0f, 2.0f)); 

	mbInitializing = true;
	mLevel = 1;
}

void KurveGame::VChangeState(BaseGameState state) {
	BaseGameLogic::VChangeState(state);

	switch(state) {
		case BGS_MainMenu:
			//DestroyLevel();
			VClear();
			mState = state;

			VAddView(K_new MainMenuView());
			break;

		case BGS_Won:
			SaveScore();

			break;
	}
}

void KurveGame::LoadLevel(int level) {
	if(level > 10 || level < 2)
		assert(0 && "ungültiges level");

	DestroyLevel();

	switch(level) {
		case 2:
			CreateBlockRow("   222   ", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("  2 5 2  ", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow(" 2 363 2 ", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("111111111", Ogre::Vector3(-8.0f, 0.0f, 2.0f));
			CreateBlockRow("   1 1   ", Ogre::Vector3(-8.0f, 0.0f, 2.0f));
			break;

		case 3:
			CreateBlockRow("35  6  43", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("   131   ", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("1 12121 1", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("212 2 212", Ogre::Vector3(-8.0f, 0.0f, 2.0f)); 
			break;

		case 4:
			CreateBlockRow("111111111", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("1   2   1", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("1 2 2 2 1", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("1 2 3 2 1", Ogre::Vector3(-8.0f, 0.0f, 2.0f));
			CreateBlockRow("262   252", Ogre::Vector3(-8.0f, 0.0f, 3.0f)); 
			break;

		case 5:
			CreateBlockRow("1   3   1", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow(" 1 343 1 ", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("  23 32  ", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("  22522  ", Ogre::Vector3(-8.0f, 0.0f, 2.0f)); 
			break;
			
		case 6:
			CreateBlockRow("1 2 2 2 1", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("5       6", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("13 3 3 31", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("4 4 4 4 4", Ogre::Vector3(-8.0f, 0.0f, 2.0f)); 
			break;

		case 7:
			CreateBlockRow("112222211", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("1   6   1", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("1 2 5 2 1", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("1 23332 1", Ogre::Vector3(-8.0f, 0.0f, 2.0f)); 
			CreateBlockRow("1  2 2  1", Ogre::Vector3(-8.0f, 0.0f, 3.0f)); 
			break;

		case 8:
			CreateBlockRow("111111111", Ogre::Vector3(-8.0f, 0.0f, -2.0f));
			CreateBlockRow("1   1   2", Ogre::Vector3(-8.0f, 0.0f, -1.0f));
			CreateBlockRow("12 1 1 22", Ogre::Vector3(-8.0f, 0.0f, 0.0f));
			CreateBlockRow("1       2", Ogre::Vector3(-8.0f, 0.0f, 1.0f));
			CreateBlockRow("5   1 1 2", Ogre::Vector3(-8.0f, 0.0f, 2.0f));	
			CreateBlockRow("    1 6 2", Ogre::Vector3(-8.0f, 0.0f, 3.0f));
			CreateBlockRow("      1 2", Ogre::Vector3(-8.0f, 0.0f, 4.0f));
			CreateBlockRow("    1   2", Ogre::Vector3(-8.0f, 0.0f, 5.0f));
			CreateBlockRow(" 44444444", Ogre::Vector3(-8.0f, 0.0f, 6.0f));
			break;
	}

	// create ball
	HActor hBall = VAddActor("Ball", AT_Ball);
	Ball *pBall = static_cast<Ball*>(VGetActor(hBall));
	pBall->mbGrabbed = true;
	pBall->mhPaddle = mhPaddle;
	pBall->VSetPosition(Ogre::Vector3(0.0f, -0.20f, -0.25f));

	EvtData_New_Actor *pRequestActor = static_cast<EvtData_New_Actor*>(g_pEventStack->alloc(sizeof(EvtData_New_Actor), ALIGNOF(EvtData_New_Actor)));
	pRequestActor->EvtData_New_Actor::EvtData_New_Actor(hBall);
	safeQueEvent(pRequestActor); 
}

void KurveGame::DestroyLevel() {
	// destroy blocks
 	blockList::iterator itBegin = mBlockList.begin();
	blockList::iterator itEnd = mBlockList.end();

	while(itBegin != itEnd) {
		EvtData_Destroy_Actor *pEvt = static_cast<EvtData_Destroy_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Destroy_Actor), ALIGNOF(EvtData_Destroy_Actor)));
		pEvt->EvtData_Destroy_Actor::EvtData_Destroy_Actor((*itBegin));
		safeQueEvent(pEvt);

		itBegin++;
	}

	// destroy ball
	if(!mhBall.IsNull()) {
		EvtData_Destroy_Actor *pEvt = static_cast<EvtData_Destroy_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Destroy_Actor), ALIGNOF(EvtData_Destroy_Actor)));
		pEvt->EvtData_Destroy_Actor::EvtData_Destroy_Actor(mhBall);
		safeTriggerEvent(pEvt);
	}
}

// Create one block row
void KurveGame::CreateBlockRow(char *pcBlocks, Ogre::Vector3 &startPos) {
	Ogre::Vector3 pos = startPos;

	static UINT nIndex = 0;

	for(DWORD x = 0; x < strlen(pcBlocks); x++) {
		if(pcBlocks[x] != ' ') {
			std::string sName = std::string("Block") + std::to_string(static_cast<long double>(nIndex));
			HActor hBlock = VAddActor(const_cast<char*>(sName.c_str()), AT_Block);
			Block *pBlock = static_cast<Block*>(VGetActor(hBlock));

			if(pcBlocks[x] == '1') pBlock->mLifes = 1;
			else if(pcBlocks[x] == '2') pBlock->mLifes = 2;
			else if(pcBlocks[x] == '3') pBlock->mLifes = 3;
			else if(pcBlocks[x] == '4') pBlock->mLifes = 4;
			else if(pcBlocks[x] == '5') { pBlock->mLifes = 1; pBlock->mBE = Block::BE_Paddle; }
			else if(pcBlocks[x] == '6') { pBlock->mLifes = 1; pBlock->mBE = Block::BE_Points; }

			pBlock->VSetPosition(pos);
			EvtData_New_Actor *pEvt = static_cast<EvtData_New_Actor*>(g_pEventStack->alloc(sizeof(EvtData_New_Actor), ALIGNOF(EvtData_New_Actor)));
			pEvt->EvtData_New_Actor::EvtData_New_Actor(hBlock);
			safeQueEvent(pEvt);

			++nIndex;
		}

		pos.x += 2.0f;
	}
}


void KurveGame::CheckForCollisions(blockList::iterator &itBegin) {
	// check for collisions witch blocks
	if(!mhBall.IsNull()) {
		// block?
		IActor *pAct = VGetActor((*itBegin));
		IActor   *pBall = VGetActor(mhBall);
		if(pAct->VGetType() == AT_Block) {
			Ogre::Vector3 block = pAct->VGetPosition();
			Ogre::Vector3 ball = pBall->VGetPosition();
			
			if(ball.x >= block.x - 1.25f &&
			   ball.x <= block.x + 1.25f &&
			   ball.z <= block.z + 0.75f &&
			   ball.z >= block.z - 0.75f) {
				   // distances to the sides
				   float fLeft = fabsf(ball.x - (block.x - 1.25f));
				   float fRight = fabsf(ball.x - (block.x + 1.25f));
				   float fTop = fabsf(ball.z - (block.z - 0.75f));
				   float fBot = fabsf(ball.z - (block.z + 0.75f));

				   float fMin = std::min(fLeft, std::min(fRight, std::min(fTop, fBot)));
				   Ogre::Vector3 ballVel = pBall->VGetVelocity();
				   if(fMin == fLeft || fMin == fRight) { // Left or Right
					   pBall->VSetVelocity(Ogre::Vector3(-1.0f * ballVel.x, ballVel.y, ballVel.z));
					   if(fMin == fLeft)
						   pBall->VSetPosition(Ogre::Vector3(ball.x - 0.2f, ball.y, ball.z));
					   else
						   pBall->VSetPosition(Ogre::Vector3(ball.x + 0.2f, ball.y, ball.z));
				   } else { // Top or Bottom
					   pBall->VSetVelocity(Ogre::Vector3(ballVel.x, ballVel.y, -1.0f * ballVel.z));

					   if(fMin == fTop)
						   pBall->VSetPosition(Ogre::Vector3(ball.x, ball.y, ball.z - 0.2f));
					   else
						   pBall->VSetPosition(Ogre::Vector3(ball.x, ball.y, ball.z + 0.2f));
				   }

				   EvtData_Collision *pEvtCollision = static_cast<EvtData_Collision*>(g_pEventStack->alloc(sizeof(EvtData_Collision), ALIGNOF(EvtData_Collision)));
				   pEvtCollision->EvtData_Collision::EvtData_Collision(CT_Ball_Box, mhBall, (*itBegin));
				   safeQueEvent(pEvtCollision);
			}
		}
	} // check for collision
}

void KurveGame::RenderText() {
	TextRenderer::getSingleton().printf("Lifes", "Lifes: %d", mTriesLeft);
	TextRenderer::getSingleton().printf("Level", "Level: %d", mLevel);
	TextRenderer::getSingleton().printf("FPS", "FPS: %f", 1.0f/g_fRealTime);
	TextRenderer::getSingleton().printf("Stack", "Stack: %.2f", g_pApp->mfFreeSpaceEventStack * 100.0f);
	TextRenderer::getSingleton().printf("TimeScale", "TimeScale: %.2f", g_pClock->GetTimeScale());
	TextRenderer::getSingleton().printf("Score", "Score: %d", mScore);
	TextRenderer::getSingleton().printf("BlockScale", "BlockScale: %f", mfBlockScale);
	TextRenderer::getSingleton().printf("Milliseconds", "Milliseconds : %f", g_fMilliseconds);
}

void KurveGame::SaveScore() {
	// update highscores
	UINT index = 0;
	for(UINT i=0; i<10; i++) {
		if(mScore > mPoints[i]) 
			break;

		index++;
	}

	UINT temp = 0;
	UINT temp2 = 0;
	for(UINT j=index; j<10; j++) {
		temp2 = temp;
		temp = mPoints[j];
		mPoints[j] = temp2;
	}

	if(mScore < mPoints[9])
		return;

	mPoints[index] = mScore;

	FILE *pFile = fopen("highscore.dat", "w+");
			
	for(UINT i=0; i<10; i++) {
		fwrite(&mPoints[i], sizeof(UINT), 1, pFile);
	}

	fclose(pFile);
}