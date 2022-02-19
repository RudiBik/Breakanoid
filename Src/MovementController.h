#pragma once

class MovementController : public Ogre::WindowEventListener {
protected:
	OIS::InputManager *mpInputMgr;
	OIS::Keyboard	  *mpKeyboard;
	OIS::Mouse		  *mpMouse;
public:
	MovementController() {
		mpInputMgr = NULL;
		mpKeyboard = NULL;
		mpMouse = NULL;
	}

	~MovementController();

	void Init();

	void windowResized(Ogre::RenderWindow *rw);
	void windowClosed(Ogre::RenderWindow *rw);

	void capture();

	OIS::Mouse*    getMouse( void ) { return mpMouse; }
	OIS::Keyboard* getKeyboard( void ) { return mpKeyboard; }
};