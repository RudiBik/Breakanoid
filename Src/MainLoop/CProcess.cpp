#include "..\EngineStd.h"

#include "CProcess.h"


// ============================= CProcess =============================
CProcess::CProcess(int nType, unsigned int nOrder) : miType(nType),
	mbKill(false), mbActive(true), mbPaused(false),
	mbInitialUpdate(true), muProcessFlags(0) {

}

CProcess::~CProcess() {

}

//				VKill
void CProcess::VKill() {
	mbKill = true;
}

//				SetNext
void CProcess::SetNext(shared_ptr<CProcess> nnext) {
	mpNext = nnext;
}

//				IsAttached
bool CProcess::IsAttached() {
	return (muProcessFlags & PROCESS_FLAG_ATTACHED) ? true : false;
}

//				SetAttached
void CProcess::SetAttached(const bool wantAttached)
{
	if(wantAttached)
	{
		muProcessFlags |= PROCESS_FLAG_ATTACHED;
	}
	else
	{
		muProcessFlags &= ~PROCESS_FLAG_ATTACHED;
	}
}


// ============================= ProcessManager ======================================
CProcessManager::~CProcessManager() {
	for(ProcessList::iterator i = mProcessList.begin(); i != mProcessList.end();) {
		Detach(*(i++));
	}
}

//				UpdateProcesses
void CProcessManager::UpdateProcesses(int delta) {
	ProcessList::iterator i = mProcessList.begin();
	ProcessList::iterator end = mProcessList.end();
	shared_ptr<CProcess> pNext;

	while ( i != end )
	{
		shared_ptr<CProcess> p( *i );
		++i;

		if ( p->IsDead() )
		{
			// Check for a child process and add if exists
			pNext = p->GetNext();
			if ( pNext )
			{
				p->SetNext(shared_ptr<CProcess>((CProcess *)NULL));
				Attach( pNext );
			}
			Detach( p );
		}
		else if ( p->IsActive() && !p->IsPaused() )
		{
			p->VOnUpdate( delta );
		}
	}
}

//				Attach
void CProcessManager::Attach(shared_ptr<CProcess> pro) {
	mProcessList.push_back(pro);
	pro->SetAttached(true);
}

//				Detach
void CProcessManager::Detach(shared_ptr<CProcess> pro) {
	mProcessList.remove(pro);
	pro->SetAttached(false);
}

//				IsProcessActive
bool CProcessManager::IsProcessActive(int type) {
	for(ProcessList::iterator i = mProcessList.begin(); i != mProcessList.end(); i++) {
		if( (*i)->GetType() == type &&
			(*i)->IsDead() == false || (*i)->GetNext())
			return true;
	}
	return false;
}

//				HasProcesses
bool CProcessManager::HasProcesses() {
	return !mProcessList.empty();
}




// ============================== WaitProcess ==============================
void CWaitProcess::VOnUpdate(int delta) {
	CProcess::VOnUpdate(delta);

		mnStart += delta;
		if(mnStart >= mnEnd) {
			MessageBox(NULL, "sdlfkje", "slkfje", MB_OK);
			VKill();
		}
}