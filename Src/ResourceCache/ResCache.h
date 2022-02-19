#pragma once


class ResHandle;
class ResCache;

class Resource
{
public:
	std::string mname;

	Resource(std::string name) { mname=name; }
	virtual ResHandle *VCreateHandle(const char *buffer, unsigned int size, ResCache *pResCache); 
};

class ZipFile;

class ResourceZipFile : public IResourceFile
{
	ZipFile *mpZipFile;
	std::wstring mresFileName;

public:
	ResourceZipFile(const wchar_t *resFileName) { mpZipFile = NULL; mresFileName=resFileName; }
	virtual ~ResourceZipFile();

	virtual bool VOpen();
	virtual int VGetResourceSize(const Resource &r);
	virtual int VGetResource(const Resource &r, char *buffer);
};

class ResHandle
{
	friend class ResCache;

protected:
	Resource mresource;
	char *mbuffer;	
	unsigned int msize;

	ResCache *mpResCache;

public:
	ResHandle(Resource & resource, char *buffer, unsigned int size, ResCache *pResCache);
	virtual ~ResHandle();
	virtual int VLoad(IResourceFile *file) 
		{ return file->VGetResource(mresource, mbuffer); }

	unsigned int Size() const { return msize; } 
	char *Buffer() const { return mbuffer; }
};

typedef std::list< shared_ptr <ResHandle > > ResHandleList;			// lru list
typedef std::map<std::string, shared_ptr < ResHandle  > > ResHandleMap;		// maps indentifiers to resource data

class ResCache
{
	friend class ResHandle;

	ResHandleList mlru;								// lru list
	ResHandleMap mresources;
	IResourceFile *mfile;

	unsigned int			mcacheSize;			// total memory size
	unsigned int			mallocated;			// total memory allocated

protected:

	bool MakeRoom(unsigned int size);
	char *Allocate(unsigned int size);
	void Free(shared_ptr<ResHandle> gonner);

	shared_ptr<ResHandle> Load(Resource * r);
	shared_ptr<ResHandle> Find(Resource * r);
	void Update(shared_ptr<ResHandle> handle);

	void FreeOneResource();
	void MemoryHasBeenFreed(unsigned int size);

public:
	ResCache(const unsigned int sizeInMb, IResourceFile *file);
	virtual ~ResCache();

	bool Init() { return mfile->VOpen(); }
	shared_ptr<ResHandle> GetHandle(Resource * r);

	void Flush(void);

};



