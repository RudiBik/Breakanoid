#pragma once

#include "..\GameApp.h"
#include "..\EventManager\EventManager.h"

class Block;

typedef std::list<HActor> blockList;


class KurveGame : public BaseGameLogic {
	friend class KurveEventListener;

protected:
	float	mfLifeTime;
	int		mTriesLeft;
	int		mLevel;
	bool	mbInitializing;

	float	mfScoreMultiplicator;
	float	mfBlockScale;
	UINT	mScore;

	// Highscore
	UINT	mPoints[10];

	IEventListenerPtr mpKurveListener;
	
	void RenderText();
	void CheckForCollisions(blockList::iterator &itBegin);
	void LoadLevel(int level);
	void DestroyLevel();
public:
	HActor mhPaddle;
	HActor mhBall;
	blockList mBlockList;

	KurveGame();
	virtual ~KurveGame();
	virtual void VClear();
	virtual void VDestroy();
	virtual void VInitialize();

	virtual bool VLoadGame(std::string name);
	virtual void VBuildInitialScene();
	virtual void VChangeState(BaseGameState state);

	void CreateBlockRow(char *pcBlocks, Ogre::Vector3 &startPos);
	UINT GetScore() { return mScore; }
	UINT *GetHighScore() { return &mPoints[0]; }
	void SaveScore();

	virtual void VOnUpdate();
};