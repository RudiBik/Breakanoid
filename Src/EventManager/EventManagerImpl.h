#pragma once

#include "EventManager.h"
#include "StackAllocator.h"

typedef std::vector<IEventListenerPtr> EventListenerVec;
typedef std::vector<EventType> EventTypeVec;

class EventManager : public IEventManager {
public:
	explicit EventManager(char *pName, bool bSetAsGlobal);
	~EventManager();

	bool VAddListener(IEventListenerPtr &inHandler, EventType &inType);
	bool VDelListener(IEventListenerPtr &inHandler, EventType &inType);
	bool VTrigger(IEventData *inEvent);
	bool VQueueEvent(IEventData *inEvent);
	bool VAbortEvent(EventType &inType, bool bAllOfType = false);
	bool VTick();
	bool VValidateType(EventType &inType);

	EventListenerVec GetListenerVector(EventType &inType);
	EventTypeVec GetTypeVector();

	void RegisterEvent(const EventType &inEvent);

private:
	
	typedef std::set<EventType> EventTypeSet;
	typedef std::pair<EventTypeSet::iterator, bool> EventTypeSetIRes;

	typedef std::list<IEventListenerPtr> EventListenerTable;

	typedef std::map<unsigned int, EventListenerTable> EventListenerMap;
	typedef std::pair<unsigned int, EventListenerTable> EventListenerMapEnt;
	typedef std::pair<EventListenerMap::iterator, bool> EventListenerMapIRes;

	typedef std::list<IEventData*> EventQueue;

	enum eConstants { kNumQueues=2 };

	EventTypeSet mtypeSet;
	EventListenerMap mregistry;
	EventQueue mqueues[kNumQueues];
	int mActiveQueue;
};






class EventSnooper : public IEventListener {
public:
	EventSnooper(char *pFileName);
	virtual ~EventSnooper();

	char* GetName() { return "Snooper"; }

	bool HandleEvent(IEventData *eventData);

private:
	FILE *mpFile;
	char mmsgBuffer[4090];
};