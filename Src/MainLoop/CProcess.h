#pragma once

class CProcessManager;

//				Process

enum PROCESS_TYPE {
	PROC_NONE,
	PROC_WAIT,
	PROC_PADDLEEFFECT,
	PROC_TEXT,
	PROC_DOUBLEPOINTS
};

// FLAGS
static const int PROCESS_FLAG_ATTACHED		= 0x00000001;

class CProcess : boost::noncopyable {
	friend class CProcessManager;

protected:
	unsigned int muProcessFlags;
	int			miType;
	bool		mbKill;
	bool		mbActive;
	bool		mbPaused;
	bool		mbInitialUpdate;
	shared_ptr<CProcess> mpNext;

public:
	CProcess(int nType, unsigned int nOrder = 0);
	virtual ~CProcess();

	bool IsDead() { return mbKill; }

	int GetType() { return miType; }
	void SetType(int t) { miType = t; }

	bool IsActive() { return mbActive; }
	void SetActive(bool b) { mbActive = b; }

	bool IsAttached();
	void SetAttached(bool wantAttached);

	bool IsPaused() { return mbPaused; }
	bool IsInitialized() { return mbInitialUpdate; }

	shared_ptr<CProcess> const GetNext() const { return mpNext; }
	void SetNext(shared_ptr<CProcess> next);

	virtual void VOnUpdate(int deltaMilliseconds);
	virtual void VOnInitialize() {}
	virtual void VKill();
	virtual void VTogglePause() { mbPaused = !mbPaused; }

};

inline void CProcess::VOnUpdate(int deltaMilli) {
	if(mbInitialUpdate) {
		VOnInitialize();
		mbInitialUpdate = false;
	}
}


//				ProcessManager
typedef std::list<shared_ptr<CProcess>> ProcessList;

class CProcessManager {
protected:
	ProcessList mProcessList;

private:
	void Detach(shared_ptr<CProcess> p);

public:
	~CProcessManager();

	void UpdateProcesses(int deltaMilliseconds);
	bool IsProcessActive( int nType );
	void Attach( shared_ptr<CProcess> pProcess );
	bool HasProcesses();
};





//				WaitProcess
class CWaitProcess : public CProcess {
protected:
	UINT			mnStart;
	UINT			mnEnd;

public:
	CWaitProcess(UINT nMilli) : CProcess(PROC_WAIT), mnStart(0), mnEnd(nMilli) {}
	virtual void VOnUpdate(int delta);
};