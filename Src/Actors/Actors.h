#pragma once
// BASEACTOR
// ACTORMANAGER

// TODO Rotationvector -> Quaternion


class BaseActor : public IActor {
protected:
	//matrix
	ActorId			mId;
	int				mType;

	// Orientation
	Ogre::Quaternion mOrientation;
	Ogre::Vector3	 mPosition;
	Ogre::Vector3	 mScale;

	Ogre::Vector3	mVelocity;
	Ogre::Vector3	mRotationVector;

public:
	BaseActor(int type, Ogre::Vector3 &pPosition, Ogre::Quaternion &pOrientation, Ogre::Vector3 &pScale)
	{
		mType = type;
		mOrientation = pOrientation;
		mPosition = pPosition;
		mScale = pScale;

		mVelocity = mRotationVector = Ogre::Vector3::ZERO;

		mId = -1;
	}

	virtual ActorId VGetId()			{ return mId; }
	virtual void	VSetId(ActorId id)  { mId = id; }
	virtual int		VGetType()			{ return mType; }
	
	virtual Ogre::Quaternion& VGetOrientation() { return mOrientation; }
	virtual Ogre::Vector3&	  VGetPosition()	{ return mPosition; }
	virtual Ogre::Vector3&	  VGetScale()		{ return mScale; }
	virtual Ogre::Vector3&	  VGetVelocity()	{ return mVelocity; }
	virtual Ogre::Vector3&	  VGetRotationVector()	{ return mRotationVector; }

	virtual void VSetPosition(const Ogre::Vector3 &pPos)			{ mPosition = pPos; }
	virtual void VSetOrientation(const Ogre::Quaternion &pOrient)   { mOrientation = pOrient; }
	virtual void VSetScale(const Ogre::Vector3 &pScale)				{ mScale = pScale; }
	virtual void VSetVelocity(const Ogre::Vector3 &pVel)		    { mVelocity = pVel; }
	virtual void VAddVelocity(const Ogre::Vector3 &pVel)			{ mVelocity += pVel; }
	virtual void VSetRotationVector(const Ogre::Vector3 &pRot)		{ mRotationVector = pRot; }

	virtual bool VIsPhysical() { return true; }
	virtual bool VIsGeometrical() { return true; }
	virtual void VOnUpdate(int deltaMilli) {}
};

#define VIEWID_NOVIEWATTACHED (0)

// Actor Handler
struct tagActor {};
typedef Handle<tagActor> HActor;

// Actor Manager
class ActorMgr : public TSingleton<ActorMgr> {
	struct Actor {
		std::string mName;
		int			mType;
		IActor	   *mpActor;

		Actor();
		bool Load(const std::string &name, int type, IActorFactory *pFac);
		void Unload(IActorFactory *pFac);
	};

	typedef HandleMgr<Actor, HActor> AHandleManager;

	// sort by name
	struct istring_less {
		bool operator () ( const std::string &l, const std::string &r ) const {
			return (::stricmp(l.c_str(), r.c_str()) < 0 ); }
	};

	typedef std::map<std::string, HActor, istring_less> NameIndex;
	typedef std::pair<NameIndex::iterator, bool>		NameIndexInsertResult;

	// data
	AHandleManager mActors;
	NameIndex	   mNameIndex;
	IActorFactory *mpActorFactory;

public:
	ActorMgr(IActorFactory *pFac);
	ActorMgr() { mpActorFactory = NULL; }
	~ActorMgr();

	void SetFactory(IActorFactory *pFac) { mpActorFactory = pFac; }

	HActor CreateActor(char *name, int type);
	void   RemoveActor(HActor &hActor);
	void   RemoveAllActors();

	int const GetType(HActor &handle) const {
		const Actor *pAc = mActors.Dereference(handle);
		if(pAc)
			return (pAc->mType);
		else return AT_Unknown;
	}
	std::string GetName(HActor &handle) const { 
		const Actor *pAc = mActors.Dereference(handle);
		if(pAc)
			return (pAc->mName);
		else return "";		 
	}
	IActor* const GetActor(HActor &handle) const { 
		const Actor *pAc = mActors.Dereference(handle);
		if(pAc)
			return (pAc->mpActor);
		else return NULL;
	}

	Ogre::SceneNode* GetSceneNode(HActor &handle, Ogre::SceneManager *pMgr) {
		IActor *pAc = GetActor(handle);
		return static_cast<Ogre::SceneNode*>(mpActorFactory->VCreateSceneNode(pAc, pMgr, pAc->VGetType()));
	}
};

#define gActorMgr ActorMgr::Get()