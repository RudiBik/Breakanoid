#pragma once

#include "..\GameApp.h"

class KurveApp : public GameApp {
protected:
	virtual BaseGameLogic* VCreateGameAndView();

	void RegisterGameSpecificEvents();

public:
	KurveApp() : GameApp() {}

	virtual WCHAR *VGetGameTitle() { return L"Kurve"; }
	virtual WCHAR *VGetGameDirectory() { return L"game code complete\\Bin"; }
	virtual HICON  VGetIcon();	
};