#pragma once

#include "EventManager.h"
#include "../Actors/Actors.h"


// Create new Actor
class EvtData_New_Actor : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_New_Actor(HActor &handle) : mHandle(handle)   {}

	HActor mHandle;
};

// Create new Actor
class EvtData_Destroy_Actor : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_Destroy_Actor(HActor &handle) : mHandle(handle)
	{
	}


	HActor mHandle;
};

class EvtData_New_Game : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }				// New_Game

	EvtData_New_Game() : BaseEventData(0.0f)
	{
	}
};

class EvtData_Request_Start_Game : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }				// Request_Start_Game

	EvtData_Request_Start_Game(IGameView *pView)
	{
		mpView = pView;
	}

	~EvtData_Request_Start_Game() { mpView = NULL; }

	IGameView *mpView;
};

class EvtData_Game_Paused : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               // Game_Paused

	EvtData_Game_Paused()
	{
	}
};

class EvtData_Game_Resume : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }                  // Game_Resume

	EvtData_Game_Resume()
	{
	}
};

class EvtData_GameState : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType &VGetEventType() { return sEventType; }

	EvtData_GameState(BaseGameState state) : mState(state)
	{
	}

	BaseGameState mState;
};


class EvtData_Error : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }                  // Error

	EvtData_Error(char *ErrorMsg)
	{
		strcpy(buffer, ErrorMsg);
	}

	char buffer[256];
};