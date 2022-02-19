#pragma once

//				IActor & Actorparams
typedef int ActorId;
struct ActorParams;


// needs to get updated
enum ActorType {
	AT_Unknown,
	AT_Paddle,
	AT_Ball,
	AT_Block
};

struct tagActor;
typedef Handle<tagActor> HActor;

class IActor {
public:
	HActor mHandle;

	virtual ~IActor() {}

	virtual ActorId VGetId()=0;
	virtual void VSetId(ActorId id)=0;

	virtual Ogre::Quaternion& VGetOrientation()=0;
	virtual Ogre::Vector3&	  VGetPosition()=0;
	virtual Ogre::Vector3&	  VGetScale()=0;
	virtual Ogre::Vector3&	  VGetVelocity()=0;
	virtual Ogre::Vector3&	  VGetRotationVector()=0;

	virtual void VSetPosition(const Ogre::Vector3 &pPos)=0;
	virtual void VSetOrientation(const Ogre::Quaternion &pOrient)=0;
	virtual void VSetScale(const Ogre::Vector3 &pScale)=0;
	virtual void VSetVelocity(const Ogre::Vector3 &pVel)=0;
	virtual void VAddVelocity(const Ogre::Vector3 &pVel)=0;
	virtual void VSetRotationVector(const Ogre::Vector3 &pRot)=0;

	virtual int  VGetType()=0;

	virtual bool VIsPhysical()=0;
	virtual bool VIsGeometrical()=0;
	virtual void VOnUpdate(int deltaMilli)=0;
};

//				IActorFactory
class IActorFactory {
public:
	virtual IActor* VCreateActor(int type)=0;
	virtual void	VDestroyActor(IActor *pAc)=0;

	virtual void* VCreateSceneNode(IActor *pAc, Ogre::SceneManager *pMan, int type)=0;
};

//				IGameLogic
class IGamePhysics;

class IGameLogic {
public:
	virtual IActor* const VGetActor(HActor &handle)=0;
	virtual HActor VAddActor(char *chName, int type)=0;
	virtual void VRemoveActor(HActor &handle)=0;
	//virtual void VMoveActor(ActorId id, EMatrix *mat)=0;

	virtual bool VLoadGame(std::string gameName)=0;

	virtual void VOnUpdate()=0;
	virtual void VChangeState(enum BaseGameState state)=0;

	virtual IGamePhysics* VGetGamePhysics()=0;

	virtual void VBuildInitialScene()=0;
};

//				IGameView
enum GameViewType {
	GVT_Not_Attached,
	GVT_Human,
};

typedef int GameViewId;

class IGameView {
public:
	virtual void VOnUpdate(int deltaMilli)=0;

	virtual GameViewType VGetType()=0;
	virtual GameViewId VGetId()=0;
	virtual void VOnAttach(GameViewId type, ActorId id)=0;

	virtual void VBuildInitialScene()=0;

	virtual ~IGameView() {}
};

// For BaseGameLogic and more
typedef std::list<IGameView*> GameViewList;

//				IResourceFile
class Resource;
class IResourceFile {
public:
	virtual bool VOpen()=0;
	virtual int VGetResourceSize(const Resource &r)=0;
	virtual int VGetResource(const Resource &r, char *buffer)=0;
	virtual ~IResourceFile() { }
};


//				IGamePhysics
class IGamePhysics {

};