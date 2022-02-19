#pragma once

#include "../MovementController.h"
#include "../EventManager/EventManager.h"
#include "../EventManager/Events.h"
#include "../GameApp.h"
#include "../ZFXAudio.h"
#include <boost\enable_shared_from_this.hpp>

typedef std::map<UINT, Ogre::SceneNode*> SceneNodeMap;

class GameView : public HumanView {
	friend class GameViewEventListener;

protected:
	Ogre::SceneManager  *mpScene;
	Ogre::Viewport		*mpVP;
	Ogre::Camera		*mpCamera;
	Ogre::SceneNode		*mpCameraNode;
	IEventListenerPtr    mpListener;

	CEGUI::Window	*mSheet;

	SceneNodeMap  mSceneNodeMap;
	BaseGameState mState;
	bool		  mbWon;
	bool		  mbMenu;

	bool updateCEGUI(const CEGUI::EventArgs& e);
	bool ContinueButton(const CEGUI::EventArgs &e);
	bool BackButton(const CEGUI::EventArgs &e);

	// buttons
	bool mbPDown;
	bool mbESCDown;

	// Paddle/Ball
	Ogre::SceneNode *mpPaddle;
	Ogre::SceneNode *mpBall;
public:
	GameView();
	virtual ~GameView();

	Ogre::SceneNode* getCamNode()	  { return mpCameraNode; }
	Ogre::SceneManager *getSceneMgr() { return mpScene; }

	void AddSceneNode(UINT handle, Ogre::SceneNode *pNode);
	Ogre::SceneNode* GetSceneNode(UINT handle);
	void RemoveSceneNode(UINT handle);

	virtual void VOnUpdate(int delta);

	virtual void VBuildInitialScene();
	void HandleGameState(BaseGameState state);
};




class MainMenuView : public HumanView {
protected:
	CEGUI::Window *mainSheet;
	CEGUI::Window *highscoreSheet;
public:
	MainMenuView() : HumanView() {
		mainSheet = NULL;
		highscoreSheet = NULL;

		// create the sheet
		CEGUI::WindowManager &wMgr = CEGUI::WindowManager::getSingleton();
		//CEGUI::Window *sheet = wMgr.createWindow(CEGUI::String("DefaultWindow"), CEGUI::String("root"));
		CEGUI::Imageset *backImage = &CEGUI::ImagesetManager::getSingleton().createFromImageFile(CEGUI::String("background"), CEGUI::String("Menue.jpg"));

		CEGUI::Window *sheet = wMgr.createWindow(CEGUI::String("TaharezLook/StaticImage"), CEGUI::String("root"));
		sheet->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f, 0), CEGUI::UDim(0.0f, 0)));
		sheet->setSize(CEGUI::UVector2(CEGUI::UDim(1.0f, 0), CEGUI::UDim(1.0f, 0)));
		sheet->setProperty(CEGUI::String("Image"), "set:background image:full_image"); 
		sheet->setProperty("FrameEnabled", "False");

		// create the start button
		CEGUI::Window *start = wMgr.createWindow(CEGUI::String("Vanilla/Button"), CEGUI::String("root/startButton"));
		start->setText(CEGUI::String("Start"));
		start->setSize(CEGUI::UVector2(CEGUI::UDim(0.17f, 0), CEGUI::UDim(0.05f, 0)));
		start->setPosition(CEGUI::UVector2(CEGUI::UDim(0.415f, 0), CEGUI::UDim(0.5f, 0)));
		start->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MainMenuView::Start, this));
		sheet->addChildWindow(start);

		// create the options button
		CEGUI::Window *highscores = wMgr.createWindow("Vanilla/Button", "root/highscores");
		highscores->setText("Highscores");
		highscores->setSize(CEGUI::UVector2(CEGUI::UDim(0.17f, 0), CEGUI::UDim(0.05f, 0)));
		highscores->setPosition(CEGUI::UVector2(CEGUI::UDim(0.415f, 0), CEGUI::UDim(0.6f, 0)));
		highscores->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MainMenuView::Highscore, this));
		sheet->addChildWindow(highscores);

		// create the quit button
		CEGUI::Window *quit = wMgr.createWindow("Vanilla/Button", "root/quitButton");
		quit->setText("Quit");
		quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.17f, 0), CEGUI::UDim(0.05f, 0)));
		quit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.415f, 0), CEGUI::UDim(0.7f, 0)));
		quit->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MainMenuView::Quit, this));
		sheet->addChildWindow(quit);

		mainSheet = sheet;
		CEGUI::System::getSingleton().setGUISheet(sheet);
		CEGUI::MouseCursor::getSingleton().setVisible(true);
	}
	virtual ~MainMenuView() { 
		CEGUI::WindowManager::getSingleton().destroyAllWindows(); 
		CEGUI::System::getSingleton().setGUISheet(NULL); 
	}

	bool Start(const CEGUI::EventArgs &) {
		// Request new game and delete the menu view
		EvtData_Request_Start_Game *evt = static_cast<EvtData_Request_Start_Game*>(g_pEventStack->alloc(sizeof(EvtData_Request_Start_Game), ALIGNOF(EvtData_Request_Start_Game)));
		evt->EvtData_Request_Start_Game::EvtData_Request_Start_Game(this);
		safeQueEvent(evt);

		return true;
	}

	bool Highscore(const CEGUI::EventArgs &) {
		CEGUI::WindowManager &wMgr = CEGUI::WindowManager::getSingleton();
		if(highscoreSheet == NULL) {
			highscoreSheet = wMgr.loadWindowLayout("highscore.layout");
			highscoreSheet->subscribeEvent(CEGUI::Window::EventWindowUpdated, CEGUI::SubscriberSlot(&MainMenuView::HighscoreLayout, this));
			CEGUI::Window *back = wMgr.getWindow("highscoreRoot/back");
			back->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MainMenuView::Back, this));
		}

		CEGUI::System::getSingleton().setGUISheet(highscoreSheet);

		return true;
	}

	bool Quit(const CEGUI::EventArgs &) {
		g_pApp->AbortGame();
		return true;
	}

	bool Back(const CEGUI::EventArgs &) {
		CEGUI::System::getSingleton().setGUISheet(mainSheet);
		return true;
	}

	bool HighscoreLayout(const CEGUI::EventArgs &args);
};