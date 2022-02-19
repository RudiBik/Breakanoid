#pragma once

// GENERAL HANDLE
template <typename TAG>
class Handle {
private:
	union {
		enum {
			MAX_BITS_INDEX = 16,
			MAX_BITS_MAGIC = 16,

			MAX_INDEX = (1 << MAX_BITS_INDEX) - 1,
			MAX_MAGIC = (1 << MAX_BITS_MAGIC) - 1,
		};

		struct {
			unsigned mIndex : MAX_BITS_INDEX;
			unsigned mMagic : MAX_BITS_MAGIC;
		};
		unsigned int mHandle;
	};

public:
	Handle() : mHandle(0) {}

	void Init(UINT uIndex);

	UINT GetIndex() const  { return mIndex; }
	UINT GetMagic() const  { return mMagic; }
	UINT GetHandle() const { return mHandle; }
	bool IsNull() const    { return !mHandle; }

	operator UINT (void) const { return mHandle; }
};


// General HandleMgr
template <typename DATA, typename HANDLE>
class HandleMgr {
private:
	typedef std::vector <DATA>	DataV;
	typedef std::vector <UINT>	MagicV;
	typedef std::vector <UINT>	FreeV;

	DataV  mUserData;
	MagicV mMagicNumbers;
	FreeV  mFreeSlots;
public:
	HandleMgr()  {}
	~HandleMgr() {}

	DATA* Acquire(HANDLE &handle);
	void  Release(HANDLE &handle);

	DATA* Dereference(HANDLE handle);
	const DATA* Dereference(HANDLE &handle) const;

	UINT GetUsedHandleCount() const { return (mMagicNumbers.size() - mFreeSlots.size()); }
	bool HasUsedHandles() const { return !GetUsedHandleCount(); }
};


// Handleimpl
template <typename TAG>
void Handle<TAG>::Init(UINT uIndex) {
	assert( IsNull() && "Handle: Reassignment not allowed" );
	assert( uIndex <= MAX_INDEX );

	static UINT sAutoMagic = 0;
	if(++sAutoMagic > MAX_INDEX) {
		sAutoMagic = 1;
	}

	mIndex = uIndex;
	mMagic = sAutoMagic;
}

template <typename TAG>
inline bool operator != ( Handle<TAG> l, Handle<TAG> r) {
	return ( l.GetHandle() != r.GetHandle() );
}

template <typename TAG>
inline bool operator == ( Handle<TAG> l, Handle<TAG> r) {
	return ( l.GetHandle() == r.GetHandle() );
}




// General Handle Manager --------------------------------------------------------------------
template <typename DATA, typename HANDLE>
DATA* HandleMgr<DATA, HANDLE>::Acquire(HANDLE &handle) {
	UINT index;
	if(mFreeSlots.empty()) { // add new one
		index = mMagicNumbers.size();
		handle.Init(index);
		mUserData.push_back(DATA());
		mMagicNumbers.push_back(handle.GetMagic());
	} else { // use a free one
		index = mFreeSlots.back();
		handle.Init(index);
		mFreeSlots.pop_back();
		mMagicNumbers[index] = handle.GetMagic();
	}

	return &mUserData[index];
}

template <typename DATA, typename HANDLE>
void HandleMgr<DATA, HANDLE>::Release(HANDLE &handle) {
	UINT index = handle.GetIndex();

	assert(index < mUserData.size());
	assert(mMagicNumbers[index] == handle.GetMagic());

	mMagicNumbers[index] = 0;
	mFreeSlots.push_back(index);
	//handle = HANDLE();
}

template <typename DATA, typename HANDLE>
inline DATA* HandleMgr<DATA, HANDLE>::Dereference(HANDLE handle) {
	if(handle.IsNull()) return NULL;

	UINT index = handle.GetIndex();
	if( (index >= mUserData.size()) ||
		(mMagicNumbers[index] != handle.GetMagic()) ) {
		//assert(0 && "Handle::Dereference: Invalid handleindex");
		return NULL;
	}

	return &mUserData[index];
}

template <typename DATA, typename HANDLE>
inline const DATA* HandleMgr<DATA, HANDLE>::Dereference(HANDLE &handle) const {
	typedef HandleMgr <DATA, HANDLE> ThisType;
	return ( const_cast<ThisType*>(this)->Dereference(handle) );
}