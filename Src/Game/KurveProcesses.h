#pragma once

#include "../MainLoop/CProcess.h"

class CPaddleEffectProcess : public CProcess {
protected:
	float mfActualScale;
	float mfDuration;
	float mfTime;
	HActor mhPaddle;

	static const float mfPaddleScale;
public:
	CPaddleEffectProcess(float mfDur, HActor &hPad) : CProcess(PROC_PADDLEEFFECT) {
		mfDuration = mfDur;
		mhPaddle = hPad;
		mfTime = 0.0f;
		mfActualScale = 1.0f;
	}

	virtual void VOnUpdate(int deltaMilliseconds);
};

class CTextProcess : public CProcess {
protected:
	std::string mId;
	float mfDuration;
	float mfTime;

public:
	CTextProcess(float mFDur, 
        const std::string& ID,
        const std::string& text,
        Ogre::Real x, Ogre::Real y,
        Ogre::Real width, Ogre::Real height,
		DWORD flags = 0,
		const std::string& font = "Arial-16",
		const std::string& fontHeight = "16",
        const Ogre::ColourValue& color = Ogre::ColourValue(1.0, 1.0, 1.0, 1.0));

	virtual void VOnUpdate(int deltaMilli);
};


class CPointsProcess : public CProcess {
protected:
	float mfDuration;
	float mfTime;
	
	float *mpfMultiplicator;

public:
	CPointsProcess(float fDur, float *pfMult) : CProcess(PROC_DOUBLEPOINTS) {
		mfDuration = fDur;
		mfTime = 0.0f;
		mpfMultiplicator = pfMult;
	}

	virtual void VOnUpdate(int deltaMilli);
};