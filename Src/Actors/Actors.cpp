#include "../EngineStd.h"
#include "Actors.h"

ActorMgr::ActorMgr(IActorFactory *pFac) {  
	assert(pFac && "ActorMgr: pFac == NULL");
	mpActorFactory = pFac;
}

ActorMgr::~ActorMgr() {
	// release all objects
	NameIndex::iterator i, begin = mNameIndex.begin(), end = mNameIndex.end();
	for(i = begin; i != end; ++i) {
		mActors.Dereference(i->second)->Unload(mpActorFactory);	
	}

	SAFE_DELETE(mpActorFactory);
}

HActor ActorMgr::CreateActor(char *name, int type) {
	assert(mpActorFactory);

	NameIndexInsertResult rc = mNameIndex.insert( std::make_pair(name, HActor()) );
	if(rc.second) {
		Actor *ac = mActors.Acquire(rc.first->second);
		if(!ac->Load(rc.first->first, type, mpActorFactory)) {
			RemoveActor(rc.first->second);
			rc.first->second = HActor();
		} else  {
			ac->mpActor->VSetId(rc.first->second.GetHandle());
			ac->mpActor->mHandle = rc.first->second;
			int test = 5;
		}
	}

	return rc.first->second;
}

void ActorMgr::RemoveActor(HActor &handle) {
	assert(mpActorFactory);

	Actor *ac = mActors.Dereference(handle);
	if(ac != NULL) {
		mNameIndex.erase(mNameIndex.find(ac->mName));

		ac->Unload(mpActorFactory);
		mActors.Release(handle);
	}
}

void ActorMgr::RemoveAllActors() {
	// release all objects
	while(mNameIndex.size() > 0) {
		HActor handle = mNameIndex.begin()->second;
		RemoveActor(handle);
	}
}

ActorMgr::Actor::Actor() {
	mType = AT_Unknown;
	mpActor = NULL;
}

bool ActorMgr::Actor::Load(const std::string &name, int type, IActorFactory *pFac) {
	mName = name;
	mType = type;

	mpActor = pFac->VCreateActor(type);
	if(!mpActor)
		return false;

	return true;
}

void ActorMgr::Actor::Unload(IActorFactory *pFac) {
	mName.erase();
	mType = AT_Unknown;
	pFac->VDestroyActor(mpActor);
}