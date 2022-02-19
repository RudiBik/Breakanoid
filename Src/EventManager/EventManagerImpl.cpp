#include "..\EngineStd.h"

#include "..\GameApp.h"
#include "..\Math\HashedString.h"
#include "EventManagerImpl.h"
#include "Events.h"

EventManager::EventManager(char *pName, bool bSetAsGlobal) : IEventManager(pName, bSetAsGlobal), mActiveQueue(0) {
}

EventManager::~EventManager() {
	mActiveQueue = 0;
}


bool EventManager::VAddListener (
	IEventListenerPtr &inListener,
	EventType &inType )
{
	if ( ! VValidateType( inType ) )
		return false;

	// check / update type list
	
	EventTypeSet::iterator evIt = mtypeSet.find(inType);                        ///

	EventListenerMap::iterator elmIt =
		mregistry.find( inType.getHashValue() );

	if ( elmIt == mregistry.end() )
	{
		EventListenerMapIRes elmIRes = mregistry.insert(
			EventListenerMapEnt( inType.getHashValue(),
								  EventListenerTable() ) );

		if ( elmIRes.second == false )
			return false;

		if ( elmIRes.first == mregistry.end() )
			return false;

		// store it so we can update the mapped list next ...
		elmIt = elmIRes.first;
	}

	EventListenerTable & evlTable = (*elmIt).second;
	
	for ( EventListenerTable::iterator it = evlTable.begin(),
			  itEnd = evlTable.end(); it != itEnd ; it++ )
	{
		bool bListenerMatch = ( *it == inListener );

		if ( bListenerMatch )
			return false;
	}

	evlTable.push_back( inListener );

	return true;
}



bool EventManager::VDelListener(IEventListenerPtr &inListener, EventType &inType) {
	if(!VValidateType(inType))
		return false;

	bool rc = false;

	for( EventListenerMap::iterator it = mregistry.begin(), itEnd = mregistry.end();
		 it != itEnd; it++) {
			 unsigned int EventId = it->first;
			 EventListenerTable &table = it->second;

			 for(EventListenerTable::iterator it2 = table.begin(), it2End = table.end();
				 it2 != it2End; it2++) {
					 if(*it2 == inListener) {
						 table.erase(it2);

						 rc = true;

						 break;
					 }
			 }
	}

	return rc;
}


bool EventManager::VTrigger(IEventData *inEvent) {
	if(!VValidateType(inEvent->VGetEventType()))
		return false;

	EventListenerMap::iterator itAll = mregistry.find(0);

	if(itAll != mregistry.end()) {
		EventListenerTable &table = itAll->second;

		for(EventListenerTable::iterator it = table.begin(), itEnd = table.end();
			it != itEnd; it++) {
				(*it)->HandleEvent(inEvent);
		}
	}

	EventListenerMap::iterator it2 = mregistry.find(inEvent->VGetEventType().getHashValue());

	if(it2 == mregistry.end())
		return false;

	EventListenerTable &table2 = it2->second;

	bool processed = false;

	for(EventListenerTable::iterator it3 = table2.begin(), it3End = table2.end(); it3!=it3End; it3++) {
		if((*it3)->HandleEvent(inEvent))
			processed = true;
	}

	return processed;
}



bool EventManager::VQueueEvent(IEventData *inEvent) {
	assert(mActiveQueue >= 0);
 	assert(mActiveQueue < kNumQueues);

	if(!VValidateType(inEvent->VGetEventType()))
		return false;

	EventListenerMap::iterator it = mregistry.find(inEvent->VGetEventType().getHashValue());

	if(it == mregistry.end()) {
		EventListenerMap::iterator itAll = mregistry.find(0);

		if(itAll == mregistry.end())
			return false;
	}

	mqueues[mActiveQueue].push_back(inEvent);

	return true;
}



bool EventManager::VAbortEvent(EventType &inType, bool bAllOfType) {
	assert(mActiveQueue >= 0);
	assert(mActiveQueue < kNumQueues);

	if(!VValidateType(inType))
		return false;

	EventListenerMap::iterator it = mregistry.find(inType.getHashValue());

	if(it == mregistry.end())
		return false;

	bool rc = false;

	EventQueue &evtQueue = mqueues[mActiveQueue];

	for(EventQueue::iterator it = evtQueue.begin(), itEnd = evtQueue.end(); it != itEnd; it++) {
		if((*it)->VGetEventType() == inType) {
			it = evtQueue.erase(it);
			rc = true;
			if(!bAllOfType)
				break;
		} else 
			it++;
	}

	return rc;
}


bool EventManager::VTick() {
	bool processed = false;

	EventListenerMap::iterator itAll = mregistry.find(0);

	int queueToProcess = mActiveQueue;
	mActiveQueue = (mActiveQueue + 1) % kNumQueues;
	mqueues[mActiveQueue].clear();

	while(mqueues[queueToProcess].size() > 0) {
		IEventData *pEvent = mqueues[queueToProcess].front();
		mqueues[queueToProcess].pop_front();

		EventType &eventType = pEvent->VGetEventType();

		EventListenerMap::iterator itListeners = mregistry.find(eventType.getHashValue());

		if(itAll != mregistry.end()) {
			EventListenerTable &table = itAll->second;

			for(EventListenerTable::iterator it2 = table.begin(), it2End = table.end(); it2 != it2End; it2++) {
				(*it2)->HandleEvent(pEvent);
			}
		}

		if(itListeners == mregistry.end())
			continue;

		UINT kEventId = itListeners->first;
		EventListenerTable &table = itListeners->second;

		/*for(EventListenerTable::iterator it = table.begin(), itEnd = table.end(); it != itEnd; it++) {
			if((*it)->HandleEvent(pEvent))
				break;
		}*/

		for(EventListenerTable::iterator it = table.begin(), itEnd = table.end(); it != itEnd; it++) {
			if((*it)->HandleEvent(pEvent))
				processed = true;
		}

//		pEvent->~IEventData();
	}

	return true;
}



bool EventManager::VValidateType(EventType &inType) {
	if(0 == inType.getStr().length())
		return false;

	if((inType.getHashValue() == 0) && (strcmp(inType.getStr().c_str(), kpAllEventTypes) != 0))
		return false;

	EventTypeSet::iterator evIt = mtypeSet.find(inType);

	if(evIt == mtypeSet.end()) {
		assert( 0 && "Failed validation of an event type; probably not registered!");
		return false;
	}

	return true;
}



EventListenerVec EventManager::GetListenerVector(EventType &eventType) {
	if(!VValidateType(eventType))
		return EventListenerVec();

	EventListenerMap::iterator itListeners = mregistry.find(eventType.getHashValue());

	if(itListeners == mregistry.end())
		return EventListenerVec();

	EventListenerTable &table = itListeners->second;

	if(table.size() == 0)
		return EventListenerVec();

	EventListenerVec result;
	result.reserve(table.size());

	for(EventListenerTable::iterator it = table.begin(), itEnd = table.end(); it != itEnd; it++) {
		result.push_back(*it);
	}

	return result;
}


EventTypeVec EventManager::GetTypeVector () 
{
	// no entries, so sad
	if ( mtypeSet.size() == 0 )
		return EventTypeVec();

	EventTypeVec result;

	result.reserve( mtypeSet.size() );

	for ( EventTypeSet::const_iterator it = mtypeSet.begin(),
			  itEnd = mtypeSet.end(); it != itEnd; it++ )
	{
		result.push_back( *it );
	}
	
	return result;
}



void EventManager::RegisterEvent(const EventType &eventType) {
	EventTypeSet::iterator iter = mtypeSet.find( eventType );
	if ( iter != mtypeSet.end() )
	{
		assert( 0 && "Attempted to register an event type that has already been registered!" );
	}
	else
	{
		mtypeSet.insert(eventType);
	}
}






// =========================

EventSnooper::EventSnooper(char *pFileName) {
	mpFile = fopen(pFileName, "w+");
}

EventSnooper::~EventSnooper() {
	if(mpFile)
		fclose(mpFile);
}


bool EventSnooper::HandleEvent(IEventData *eventData) {
#ifdef _DEBUG
	memset( mmsgBuffer, 0, sizeof(mmsgBuffer));
	
#pragma warning( push )
#pragma warning( disable : 4313 ) 

	if(eventData->VGetEventType() == EvtData_Error::sEventType) {
		EvtData_Error &ev = (EvtData_Error&)eventData;

		_snprintf( mmsgBuffer, sizeof(mmsgBuffer)-1,
			"Event Snoop : event %08lx time %g : type %08lx [%s] : Error-Message: %s \n",
			& eventData,
			eventData->VGetTimeStamp(),
			eventData->VGetEventType().getHashValue(),
			eventData->VGetEventType().getStr().c_str(),
			ev.buffer,
			0 );
	} else {
		_snprintf( mmsgBuffer, sizeof(mmsgBuffer)-1,
			"Event Snoop : event %08lx time %g : type %08lx [%s] : \n",
			& eventData,
			eventData->VGetTimeStamp(),
			eventData->VGetEventType().getHashValue(),
			eventData->VGetEventType().getStr().c_str(),
			0 );
	}

#pragma warning( pop )

	OutputDebugStringA( mmsgBuffer );

	if ( mpFile != NULL )
	{
		fwrite( mmsgBuffer, (DWORD) strlen(mmsgBuffer), 1, mpFile );
		fflush(mpFile);
	}
#endif

	return false;
}