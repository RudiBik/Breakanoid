#include "KurveStd.h"

#include "KurveActors.h"


//========================================================================
// Paddle
//========================================================================
void Paddle::VOnUpdate(int delta) {
	mPosition += mVelocity * g_fElapsedTime;

	mVelocity *= powf(0.2f, g_fElapsedTime);

	if(mPosition.x + mScale.x >= 9.25f) { 
		mVelocity *= -0.5f;
		mPosition.x = 9.25f - mScale.x;
	} else if(mPosition.x - mScale.x <= -9.25f) {
		mVelocity *= -0.5f;
		mPosition.x = -9.25f + mScale.x;
	}

	EvtData_Move_Actor *pEvt = static_cast<EvtData_Move_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Move_Actor), ALIGNOF(EvtData_Move_Actor)));
	pEvt->EvtData_Move_Actor::EvtData_Move_Actor(mHandle, &mPosition, &mOrientation, &mScale);
	safeQueEvent(pEvt);
}

void Paddle::VSetScale(const Ogre::Vector3 &pScale) {
	mScale.x = pScale.x;

	mfWidth = mfStandartWidth * pScale.x;
}

void Paddle::SetBall(HActor &handle) {
	mbBall = true;
	mhBall = handle;
}

void Paddle::Fire() {
	mbBall = false;
	if(!mhBall.IsNull()) {
		Ball *pBall = static_cast<Ball*>(gActorMgr->GetActor(mhBall));
		pBall->Fire();
		mhBall = HActor();
	}
}


//========================================================================
// Ball
//========================================================================
void Ball::VOnUpdate(int delta) {
	bool bHitWall = false;

	if(!mbGrabbed) {
		mPosition += mVelocity * g_fElapsedTime;

		if(mPosition.x + 0.25f >= 9.25f) { mVelocity.x *= -1.0f; mPosition.x = 9.0f; bHitWall = true; } 
		else if(mPosition.x - 0.25f <= -9.25f) { mVelocity.x *= -1.0f; mPosition.x = -9.0f; bHitWall = true; }
		if(mPosition.z - 0.25f <= -4.25f) { mVelocity.z *= -1.0f; mPosition.z = -4.0f; bHitWall = true; }
		if(mPosition.z >= 12.0f) {
			// destroy the ball and return
			EvtData_MissedBall *pMissEvt = static_cast<EvtData_MissedBall*>(g_pEventStack->alloc(sizeof(EvtData_MissedBall), ALIGNOF(EvtData_MissedBall)));
			pMissEvt->EvtData_MissedBall::EvtData_MissedBall(mHandle);
			safeQueEvent(pMissEvt);

			EvtData_Destroy_Actor *pDestEvent = static_cast<EvtData_Destroy_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Destroy_Actor), ALIGNOF(EvtData_Destroy_Actor)));
			pDestEvent->EvtData_Destroy_Actor::EvtData_Destroy_Actor(mHandle);
			safeTriggerEvent(pDestEvent);

			return;
		}

		// Collision with Paddle
		Paddle *pPaddle = static_cast<Paddle*>(gActorMgr->GetActor(mhPaddle));
		Ogre::Vector3 pos = pPaddle->VGetPosition();
		float scalex = pPaddle->VGetScale().x;
		if(mPosition.x >= pos.x - scalex - 0.25f &&
		   mPosition.x <= pos.x + scalex + 0.25f &&
		   mPosition.z >= pos.z - 0.25f &&
		   mPosition.z - mVelocity.z * g_fElapsedTime <= pos.z - 0.25f) {
			   mVelocity.z *= -1.0f;
			   mVelocity.x -= pPaddle->VGetVelocity().x * 0.5f;
			   mPosition.z = pos.z - 0.25f;

			   EvtData_Collision *pColEvent = static_cast<EvtData_Collision*>(g_pEventStack->alloc(sizeof(EvtData_Collision), ALIGNOF(EvtData_Collision)));
			   pColEvent->EvtData_Collision::EvtData_Collision(CT_Paddle_Ball, mHandle, mhPaddle);
			   safeQueEvent(pColEvent);
		}
	}

	if(bHitWall) {
	    EvtData_Collision *pColWall = static_cast<EvtData_Collision*>(g_pEventStack->alloc(sizeof(EvtData_Collision), ALIGNOF(EvtData_Collision)));
		//shared_ptr<IActor> mhk = shared_ptr<Ball>(this);
		pColWall->EvtData_Collision::EvtData_Collision(CT_Ball_Wall, mHandle, mhPaddle);
		safeQueEvent(pColWall);
	}

	EvtData_Move_Actor *pEvt = static_cast<EvtData_Move_Actor*>(g_pEventStack->alloc(sizeof(EvtData_Move_Actor), ALIGNOF(EvtData_Move_Actor)));
	pEvt->EvtData_Move_Actor::EvtData_Move_Actor(mHandle, &GetAbsPos());
	safeQueEvent(pEvt);
}

void Ball::Fire() {
	if(mbGrabbed) {
		IActor *pPad = gActorMgr->GetActor(mhPaddle);
		mbGrabbed = false;
		mPosition += pPad->VGetPosition();
		mVelocity.x = g_pApp->mpGame->mRandom.Random() * 8.0f - 4.0f;
		mVelocity.y = 0.0f;
		mVelocity.z = -(g_pApp->mpGame->mRandom.Random() * 2.0f) - 8.0f;
		mVelocity += pPad->VGetVelocity();

		mPosition += 0.1f;
	}
}

Ogre::Vector3 Ball::GetAbsPos() {
	if(!mbGrabbed) {
		return mPosition;
	} else {
		IActor *pAc = gActorMgr->GetActor(mhPaddle);
		Ogre::Vector3 newPos = mPosition + pAc->VGetPosition();
		return newPos;
	}
}

