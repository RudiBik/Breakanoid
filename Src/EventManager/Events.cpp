#include "..\EngineStd.h"

#include "Events.h"

EventType EvtData_New_Game::sEventType("new_game");
EventType EvtData_Request_Start_Game::sEventType = EventType("request_start_game");
EventType EvtData_Game_Paused::sEventType("game_paused");
EventType EvtData_Game_Resume::sEventType("game_resume");
EventType EvtData_GameState::sEventType("game_state");
EventType EvtData_Error::sEventType("error");
EventType EvtData_New_Actor::sEventType("new_actor");
EventType EvtData_Destroy_Actor::sEventType("destroy_actor");