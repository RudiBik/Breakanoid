#pragma once
#include "../Actors/Actors.h"

class Ball;

//========================================================================
// Paddle
//========================================================================
class Paddle : public BaseActor {
public:
	bool		mbBall;
	HActor	    mhBall; //
	float		mfWidth;
	float		mfStandartWidth;

	Paddle(int type, Ogre::Vector3 &pPosition, Ogre::Quaternion &pOrientation, Ogre::Vector3 &pScale) 
		: BaseActor(type, pPosition, pOrientation, pScale), mbBall(false), mhBall(), mfWidth(1.0f), mfStandartWidth(1.0f) {}

	virtual void VOnUpdate(int deltaMilli);
	virtual void VSetScale(const Ogre::Vector3 &pScale);

	void SetBall(HActor &pBall);
	void Fire();
};


/*struct PaddleParams : public ActorParams {
	bool mbBall;
	shared_ptr<Ball> mpBall;
	float mWidth;
	float mStandartWidth;

	PaddleParams();

	virtual bool VInit(std::istrstream &str);
	virtual void VSerialize(std::ostrstream &out);

	shared_ptr<IActor> VCreate(IGameLogic *logic);
	Ogre::SceneNode* VCreateSceneNode(Ogre::SceneManager *pMgr);
};*/


//========================================================================
// Ball
//========================================================================
class Ball : public BaseActor {
public:
	bool mbGrabbed;
	HActor mhPaddle;

	Ball(int type, Ogre::Vector3 &pPosition, Ogre::Quaternion &pOrientation, Ogre::Vector3 &pScale) 
		: BaseActor(type, pPosition, pOrientation, pScale), mbGrabbed(false), mhPaddle() {
	
	}

	virtual void VOnUpdate(int delta);
	void Attach(HActor mhPaddle);
	void Fire();
	Ogre::Vector3 GetAbsPos();
};

/*struct BallParams : public ActorParams {
	bool mbGrabbed;
	Paddle *mpPaddle;

	BallParams();

	shared_ptr<IActor> VCreate(IGameLogic *logic);
	Ogre::SceneNode* VCreateSceneNode(Ogre::SceneManager *pMgr);
};
*/

//========================================================================
// Block
//========================================================================
class Block : public BaseActor {
public:
	Block(int type, Ogre::Vector3 &pPosition, Ogre::Quaternion &pOrientation, Ogre::Vector3 &pScale)
		: BaseActor(type, pPosition, pOrientation, pScale), mLifes(1), mBE(BE_None) {}

	enum BlockEffect {
		BE_None,
		BE_Paddle,
		BE_Points
	};

	int mLifes;
	BlockEffect mBE;

};

/*struct BlockParams : public ActorParams {
	enum BlockEffect {
		BE_None,
		BE_Paddle,
		BE_Points
	};

	int mLifes;
	BlockEffect mBE;

	BlockParams();

	shared_ptr<IActor> VCreate(IGameLogic *logic);
	Ogre::SceneNode* VCreateSceneNode(Ogre::SceneManager *pMgr);
	};*/

class ActorFactory : public IActorFactory {
public:
	virtual IActor* VCreateActor(int type) {
		IActor *pAc = NULL;

		switch(type) {
		case AT_Paddle:
			pAc = K_new Paddle(type, Ogre::Vector3(0.0f), Ogre::Quaternion(), Ogre::Vector3(1.0f));
			break;

		case AT_Ball:
			pAc = K_new Ball(type, Ogre::Vector3(0.0f), Ogre::Quaternion(), Ogre::Vector3(1.0f));
			break;

		case AT_Block:
			pAc = K_new Block(type, Ogre::Vector3(0.0f), Ogre::Quaternion(), Ogre::Vector3(1.0f));
			break;

		default: {
			assert(0 && "Factory: Unknown actor type");
			return NULL;
				 }
		}

		return pAc;
	}
	virtual void	VDestroyActor(IActor *pAc) {
		SAFE_DELETE(pAc);
	}

	virtual void* VCreateSceneNode(IActor *pActor, Ogre::SceneManager *pMgr, int type) {
		Ogre::SceneNode *pNode;
		Ogre::Entity *pEnt;
		Block *pAc;
		Ball *pBall;

		switch(type) {
		case AT_Paddle:
			pEnt = pMgr->createEntity("paddle.mesh");
			pEnt->setCastShadows(true);
			pNode = pMgr->getRootSceneNode()->createChildSceneNode();
			pNode->attachObject(pEnt);
			pNode->translate(pActor->VGetPosition(), Ogre::Node::TS_WORLD);
			break;

		case AT_Ball:
			pBall = static_cast<Ball*>(pActor);
			pEnt = pMgr->createEntity("ball.mesh");
			pEnt->setCastShadows(true);
			pNode = pMgr->getRootSceneNode()->createChildSceneNode();
			pNode->attachObject(pEnt);
			pNode->translate(pBall->GetAbsPos(), Ogre::Node::TS_WORLD);
			break;

		case AT_Block:
			pAc = static_cast<Block*>(pActor);
			if(pAc->mBE != Block::BE_None) {
				if(pAc->mBE == Block::BE_Paddle) 
					pEnt = pMgr->createEntity("blockBEPaddle.mesh");
				else if(pAc->mBE == Block::BE_Points)
					pEnt = pMgr->createEntity("blockBEPoints.mesh");
				else {
					assert(0 && "VCreateSceneNode: Unknown BlockEffect");
					return NULL;
				}
			} else {
				if(pAc->mLifes == 1) 
					pEnt = pMgr->createEntity("block1.mesh");
				else if(pAc->mLifes == 2)
					pEnt = pMgr->createEntity("block2.mesh");
				else if(pAc->mLifes == 3)
					pEnt = pMgr->createEntity("block3.mesh");
				else
					pEnt = pMgr->createEntity("block4.mesh");
			}

			pEnt->setCastShadows(true);
			pNode = pMgr->getRootSceneNode()->createChildSceneNode();
			pNode->attachObject(pEnt);
			pNode->translate(pActor->VGetPosition(), Ogre::Node::TS_WORLD);
			break;

		default: {
			assert(0 && "Unknown actor");
			return NULL;
				 }
		}

		return pNode;
	}

	//void VRemoveSceneNode(
};