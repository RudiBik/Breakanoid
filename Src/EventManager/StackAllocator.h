#pragma once

typedef UINT Marker;

class StackAllocator {
public:
	explicit StackAllocator();
	~StackAllocator();


	bool init(UINT nBytesToAllocate);
	void* alloc(UINT nBytes, UINT alignment); // returns aligned pointer
	
	void	clear();
	Marker  getMarker();				// marker of the actual position
	void	freeToMarker(Marker pos);
	
	float getFreeSpace() { return static_cast<float>(mnAllocatedBytes); }

private:
	void* alloc(UINT nBytes);			// returns unaligned pointer

	UINT     mnSizeBytes;
	bool     mbInitialised;
	void	*mpStart;

	UINT	 mnAllocatedBytes;	
	void	*mpActPos;
	Marker   mnActPos;
};


class DoubleBufferedAllocator {
public:
	DoubleBufferedAllocator() {}

	bool init(UINT nBytesPerBuffer) {
		mnSizeOfBuffers = nBytesPerBuffer*2;
		if(!mstack[0].init(nBytesPerBuffer) || !mstack[1].init(nBytesPerBuffer))
			return false;

		mcurStack = 0;
		return true;
	}

	void* alloc(UINT nBytes, UINT nAlignment) {
		return mstack[mcurStack].alloc(nBytes, nAlignment);
	}

	void swapBuffers() {
		mcurStack = (UINT)!mcurStack;
	}

	void clearCurrentBuffer() {
		mstack[mcurStack].clear();
	}

	float getFreeSpace() { return (mnSizeOfBuffers - (mstack[0].getFreeSpace() + mstack[1].getFreeSpace())) / (float)(mnSizeOfBuffers); }
private:
	StackAllocator mstack[2];
	short		   mcurStack;

	UINT		   mnSizeOfBuffers;
};
extern DoubleBufferedAllocator *g_pEventStack;