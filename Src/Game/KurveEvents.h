#pragma once

#include "../EventManager/Events.h"
#include "../Actors/Actors.h"


// Move Actor
class EvtData_Move_Actor : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_Move_Actor(HActor &handle, Ogre::Vector3 *pPos, Ogre::Quaternion *pOrient = &Ogre::Quaternion(), Ogre::Vector3 *pScale = &Ogre::Vector3(1.0f)) 
	{
		mHandle = handle;
		mPos = *pPos;
		mOrient = *pOrient;
		mScale = *pScale;
	}

	HActor			 mHandle;
	Ogre::Vector3	 mPos;
	Ogre::Quaternion mOrient;
	Ogre::Vector3	 mScale;
};

// Player hits left/right button
class EvtData_Steer : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_Steer(float fSteer)
	{
		mfSteer = fSteer;
	}

	float mfSteer;
};

// Player hits space when the ball is clued to the paddle
class EvtData_Fire : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_Fire()
	{
	}
};

// Ball leaves the area
class EvtData_MissedBall : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_MissedBall(HActor &handle) : mHandle(handle)
	{
	}

	HActor mHandle;
};

// collision event
enum CollisionType {
	CT_Paddle_Wall,
	CT_Paddle_Ball,
	CT_Ball_Wall,
	CT_Ball_Box
};

class EvtData_Collision : public BaseEventData {
public:
	static EventType sEventType;
	virtual EventType& VGetEventType() { return sEventType; }               

	EvtData_Collision(CollisionType typeOfCollision, HActor &handle1, HActor &handle2)
	{
		type = typeOfCollision;
		mHandle1 = handle1;
		mHandle2 = handle2;
	}

	CollisionType type;
	HActor mHandle1;
	HActor mHandle2;
};

