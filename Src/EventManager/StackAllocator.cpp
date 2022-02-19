#include "..\EngineStd.h"
#include "StackAllocator.h"

DoubleBufferedAllocator *g_pEventStack = NULL;

StackAllocator::StackAllocator() {
	mpStart = NULL;
	mpActPos = NULL;
	mnActPos = 0;
	mnAllocatedBytes = 0;
	mbInitialised = false;
}

StackAllocator::~StackAllocator() {
	if(mpStart) {
		free(mpStart);
		mpStart = NULL;
		mpActPos = NULL;
		mnActPos = 0;
		mbInitialised = false;
	}
}

bool StackAllocator::init(UINT nBytesToAllocate) {
	assert(nBytesToAllocate > 16 && "nBytesToAllocate < 16 bytes.");
	mnSizeBytes = nBytesToAllocate;

	mpStart = malloc(mnSizeBytes);
	if(!mpStart) {
		ERR("Not enough memory!");
		return false;
	}

	mpActPos = mpStart;
	mnActPos = reinterpret_cast<Marker>(mpStart);
	mbInitialised = true;

	return true;
}

void* StackAllocator::alloc(UINT nBytes, UINT alignment) {
	assert(mbInitialised && "StackAllocator not initialised!");
	assert((mnAllocatedBytes + nBytes + alignment) < mnSizeBytes && "StackAllocater has no more room.");
	assert(alignment > 1 && "StackAllocator: alignment < 1");

	UINT expandedSize = nBytes + alignment;

	UINT rawAddress = reinterpret_cast<UINT>(alloc(expandedSize));

	UINT mask = (alignment - 1);
	UINT misalignment = (rawAddress & mask);
	UINT adjustment = alignment - misalignment;

	UINT alignedAddress = rawAddress + adjustment;
	return reinterpret_cast<void*>(alignedAddress);
}

// unaligned
void* StackAllocator::alloc(UINT nBytes) {
	void *pReturnPointer = mpActPos;

	// pointer verschieben
	mnActPos += nBytes;
	mpActPos = reinterpret_cast<void*>(mnActPos);
	mnAllocatedBytes += nBytes;

	return pReturnPointer;
}
	
void StackAllocator::clear() {
	assert(mbInitialised && "StackAllocator not initialised!");

	mpActPos = mpStart;
	mnActPos = reinterpret_cast<Marker>(mpStart);
	mnAllocatedBytes = 0;
}

Marker StackAllocator::getMarker() {
	assert(mbInitialised && "StackAllocator not initialised!");

	return mnActPos;
}

void StackAllocator::freeToMarker(Marker pos) {
	assert(mbInitialised && "StackAllocator not initialised!");

	UINT nFreedMemory = mnActPos - pos;
	mnAllocatedBytes -= nFreedMemory;

	mpActPos = reinterpret_cast<void*>(pos);
	mnActPos = pos;
}