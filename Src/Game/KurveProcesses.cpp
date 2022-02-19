#include "KurveStd.h"

#include "KurveProcesses.h"

const float CPaddleEffectProcess::mfPaddleScale = 2.0f;

// Paddle effect
void CPaddleEffectProcess::VOnUpdate(int deltaMilliseconds) {
	CProcess::VOnUpdate(deltaMilliseconds);

	mfTime += static_cast<float>(g_fElapsedTime);
	mfActualScale +=  static_cast<float>(5.0f * g_fElapsedTime);
	if(mfActualScale > mfPaddleScale)
		mfActualScale = mfPaddleScale;

	IActor *pPaddle = gActorMgr->GetActor(mhPaddle);
	if(pPaddle)
		pPaddle->VSetScale(Ogre::Vector3(mfActualScale, 1.0f, 1.0f));

	if(mfTime > mfDuration) {
		pPaddle->VSetScale(Ogre::Vector3(1.0f, 1.0f, 1.0f));
		VKill();
	}
}


// Text
CTextProcess::CTextProcess(float fDur, 
				const std::string& ID,
                const std::string& text,
                Ogre::Real x, Ogre::Real y,
                Ogre::Real width, Ogre::Real height,
				DWORD flags,
				const std::string &font,
				const std::string &fontHeight,
                const Ogre::ColourValue& color) : CProcess(PROC_TEXT) {
	mfDuration = fDur;
	mfTime = 0.0f;
	mId = ID;

	TextRenderer::getSingleton().addTextBox(ID, text, x, y, width, height, flags, font, fontHeight, color);
}

void CTextProcess::VOnUpdate(int deltaMilli) {
	CProcess::VOnUpdate(deltaMilli);

	mfTime += static_cast<float>(g_fElapsedTime);

	if(mfTime > mfDuration) {
		TextRenderer::getSingleton().removeTextBox(mId);
		VKill();
	}
}


void CPointsProcess::VOnUpdate(int delta) {
	CProcess::VOnUpdate(delta);

	mfTime += static_cast<float>(g_fElapsedTime);
	*mpfMultiplicator = 2.0f;

	if(mfTime > mfDuration) {
		*mpfMultiplicator = 1.0f;
		VKill();
	}
}