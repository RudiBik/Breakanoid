#include "EngineStd.h"
#include "GameApp.h"

#include "MovementController.h"

MovementController::~MovementController() {
    if( mpInputMgr ) {
        if( mpMouse ) {
            mpInputMgr->destroyInputObject( mpMouse );
            mpMouse = 0;
        }
 
        if( mpKeyboard ) {
            mpInputMgr->destroyInputObject( mpKeyboard );
            mpKeyboard = 0;
        }
 
        OIS::InputManager::destroyInputSystem(mpInputMgr);
        mpInputMgr = 0;
        mpInputMgr = 0;
    }

	Ogre::WindowEventUtilities::removeWindowEventListener(g_pApp->mpWindow, this);
}


void MovementController::Init() {
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
 
	g_pApp->mpWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
	mpInputMgr = OIS::InputManager::createInputSystem( pl );

	mpKeyboard = static_cast<OIS::Keyboard*>(mpInputMgr->createInputObject( OIS::OISKeyboard, true ));
	mpMouse = static_cast<OIS::Mouse*>(mpInputMgr->createInputObject( OIS::OISMouse, true ));

	windowResized(g_pApp->mpWindow);
	Ogre::WindowEventUtilities::addWindowEventListener(g_pApp->mpWindow, this);

	mpKeyboard->setEventCallback(g_pApp);
	mpMouse->setEventCallback(g_pApp);
}

void MovementController::windowResized(Ogre::RenderWindow *rw) {
    unsigned int width, height, depth;
    int left, top;
    g_pApp->mpWindow->getMetrics(width, height, depth, left, top);
 
    const OIS::MouseState &ms = mpMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

void MovementController::windowClosed(Ogre::RenderWindow *rw) {
    //Only close for window that created OIS (the main window in these demos)
	if( rw == g_pApp->mpWindow )
    {
        if( mpInputMgr )
        {
            mpInputMgr->destroyInputObject( mpMouse );
            mpInputMgr->destroyInputObject( mpKeyboard );
 
            OIS::InputManager::destroyInputSystem(mpInputMgr);
            mpInputMgr = 0;
        }
    }
}

void MovementController::capture() {
	mpMouse->capture();
	mpKeyboard->capture();
}
