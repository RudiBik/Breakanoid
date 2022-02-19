#pragma once

#include "..\Math\HashedString.h"

//				IEventData
typedef HashedString EventType;

class IEventData {
public:
	virtual ~IEventData() {}
	virtual EventType& VGetEventType()=0;
	virtual float VGetTimeStamp()=0;
	virtual void VSerialize(std::ostrstream &out)=0;
};


class BaseEventData : public IEventData {
public:
	explicit BaseEventData(float TimeStamp = 0.0f) : mfTimeStamp(TimeStamp) {}
	virtual ~BaseEventData() {}
	virtual EventType& VGetEventType()=0;
	float VGetTimeStamp() { return mfTimeStamp; }
	virtual void VSerialize(std::ostrstream &out) {}                                                    // ostrstream is very slow

protected:
	float mfTimeStamp;
};



//				IEventListener
class IEventListener {
public:
	explicit IEventListener() {}
	virtual ~IEventListener() {}

	virtual char* GetName()=0;

	virtual bool HandleEvent(IEventData *eventData)=0 {
		return true;
	}
};

typedef shared_ptr<IEventListener> IEventListenerPtr;

//				IEventManager
class IEventManager {
public:
	enum eConstants { kINFINITE = 0xffffffff };

	explicit IEventManager(char *pName, bool setAsGlobal);
	virtual ~IEventManager();

	virtual bool VAddListener(IEventListenerPtr &inHandler, EventType &inType)=0;
	virtual bool VDelListener(IEventListenerPtr &inHandler, EventType &inType)=0;
	virtual bool VTrigger(IEventData *inEvent)=0;
	virtual bool VQueueEvent(IEventData *inEvent)=0;
	virtual bool VAbortEvent(EventType &inType, bool bAllOfType = false)=0;
	virtual bool VTick()=0;
	virtual bool VValidateType(EventType &inType)=0;

private:
	static IEventManager *Get();

	friend bool safeAddListener(IEventListenerPtr &inHandler, EventType &inType);
	friend bool safeDelListener(IEventListenerPtr &inHandler, EventType &inType);
	friend bool safeTriggerEvent( IEventData *inEvent );
	friend bool safeQueEvent( IEventData  *inEvent );
	friend bool safeAbortEvent( EventType  & inType, bool allOfType = false );
	friend bool safeTickEventManager();
	friend bool safeValidateEventType( EventType  & inType );
};
extern IEventManager *g_pEventMgr;

bool safeAddListener( IEventListenerPtr  & inHandler,
					  EventType  & inType );
	
bool safeDelListener( IEventListenerPtr  & inHandler,
					  EventType  & inType );

bool safeTriggerEvent( IEventData *inEvent );

bool safeQueEvent( IEventData  *inEvent );

bool safeAbortEvent( EventType  & inType,
					 bool allOfType /* = false */ );

bool safeTickEventManager();

bool safeValidateEventType( EventType  & inType );