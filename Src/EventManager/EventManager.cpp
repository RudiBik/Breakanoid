#include "..\EngineStd.h"

#include "..\Math\HashedString.h"
#include "EventManager.h"


IEventManager *g_pEventMgr;

IEventManager* IEventManager::Get() {
	return g_pEventMgr;
}

IEventManager::IEventManager(char *pName, bool bSetAsGlobal) {
	if(bSetAsGlobal) {
		g_pEventMgr = this;
	}
}

IEventManager::~IEventManager() {
	if(g_pEventMgr == this) 
		g_pEventMgr = NULL;
}

//

bool safeAddListener( IEventListenerPtr  & inHandler, EventType  & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VAddListener( inHandler, inType );
}

bool safeDelListener( IEventListenerPtr  & inHandler, EventType  & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VDelListener( inHandler, inType );
}

bool safeTriggerEvent( IEventData  *inEvent )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VTrigger( inEvent );
}

bool safeQueEvent( IEventData  *inEvent )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VQueueEvent( inEvent );
}


bool safeAbortEvent( EventType  & inType,
					 bool allOfType /*= false*/ )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VAbortEvent( inType, allOfType );
}

bool safeTickEventManager()
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VTick();
}

bool safeValidateEventType( EventType  & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VValidateType( inType );
}


