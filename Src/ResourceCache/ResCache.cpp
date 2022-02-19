#include "..\EngineStd.h"

#include "ResCache.h"
#include "ZipFile.h"




ResHandle *Resource::VCreateHandle(const char *buffer, unsigned int size, ResCache *pResCache)
{ 
	return K_new ResHandle(*this, (char *)buffer, size, pResCache); 
}


ResourceZipFile::~ResourceZipFile() 
{ 
	SAFE_DELETE(mpZipFile); 
}

bool ResourceZipFile::VOpen()
{
	mpZipFile = K_new ZipFile;
    if (mpZipFile)
    {
		return mpZipFile->Init(mresFileName.c_str());
	}
	return false;	
}

int ResourceZipFile::VGetResourceSize(const Resource &r)
{
	int size = 0;
	int resourceNum = mpZipFile->Find(r.mname.c_str());
	if (resourceNum >= 0)
	{
		size = mpZipFile->GetFileLen(resourceNum);
	}
	return size;	
}

int ResourceZipFile::VGetResource(const Resource &r, char *buffer)
{
	int size = 0;
	int resourceNum = mpZipFile->Find(r.mname.c_str());
	if (resourceNum >= 0)
	{
		size = mpZipFile->GetFileLen(resourceNum);
		mpZipFile->ReadFile(resourceNum, buffer);
	}
	return 0;	
}



ResHandle::ResHandle(Resource & resource, char *buffer, unsigned int size, ResCache *pResCache)
: mresource(resource)
{
	mbuffer = buffer;
	msize = size;
	mpResCache = pResCache;
}

ResHandle::~ResHandle()
{
	if (mbuffer) delete [] mbuffer;
	mpResCache->MemoryHasBeenFreed(msize);
}

ResCache::ResCache(const unsigned int sizeInMb, IResourceFile *resFile )
{
	mcacheSize = sizeInMb * 1024 * 1024;				// total memory size
	mallocated = 0;									// total memory allocated
	mfile = resFile;
}

ResCache::~ResCache()
{
	while (!mlru.empty())
	{
		FreeOneResource();
	}
	SAFE_DELETE(mfile);
}




shared_ptr<ResHandle> ResCache::GetHandle(Resource * r)
{
	shared_ptr<ResHandle> handle(Find(r));
	if (handle==NULL)
	{
		handle = Load(r);
	}
	else
	{
		Update(handle);
	}
	return handle;
}

shared_ptr<ResHandle> ResCache::Load(Resource *r)
{
	int size = mfile->VGetResourceSize(*r);
	char *buffer = Allocate(size);
	if (buffer==NULL)
	{
		return shared_ptr<ResHandle>();		// ResCache is out of memory!
	}

	// Create a new resource and add it to the lru list and map
	shared_ptr<ResHandle> handle (r->VCreateHandle(buffer, size, this));
	handle->VLoad(mfile);

	mlru.push_front(handle);
	mresources[r->mname] = handle;

	return handle;
}


shared_ptr<ResHandle> ResCache::Find(Resource * r)
{
	ResHandleMap::iterator i = mresources.find(r->mname);
	if (i==mresources.end())
		return shared_ptr<ResHandle>();

	return (*i).second;
}

void ResCache::Update(shared_ptr<ResHandle> handle)
{
	mlru.remove(handle);
	mlru.push_front(handle);
}




char *ResCache::Allocate(unsigned int size)
{
	if (!MakeRoom(size))
		return NULL;

	char *mem = K_new char[size];
	if (mem)
	{
		mallocated += size;
	}

	return mem;
}


void ResCache::FreeOneResource()
{
	ResHandleList::iterator gonner = mlru.end();
	gonner--;

	shared_ptr<ResHandle> handle = *gonner;

	mlru.pop_back();							
	mresources.erase(handle->mresource.mname);
	//mallocated -= handle->mresource.msize ;
	//delete handle;
	// This is where the reshandle should be freed....
}




void ResCache::Flush()
{
	while (!mlru.empty())
	{
		shared_ptr<ResHandle> handle = *(mlru.begin());
		Free(handle);
		mlru.pop_front();
	}
}



bool ResCache::MakeRoom(unsigned int size)
{
	if (size > mcacheSize)
	{
		return false;
	}

	// return null if there's no possible way to allocate the memory
	while (size > (mcacheSize - mallocated))
	{
		// The cache is empty, and there's still not enough room.
		if (mlru.empty())
			return false;

		FreeOneResource();
	}

	return true;
}



void ResCache::Free(shared_ptr<ResHandle> gonner)
{
	mlru.remove(gonner);
	mresources.erase(gonner->mresource.mname);
	// Note - the resource might still be in use by something,
	// so the cache can't actually count the memory freed until the
	// ResHandle pointing to it is destroyed.

	//mallocated -= gonner->mresource.msize;
	//delete gonner;
}

void ResCache::MemoryHasBeenFreed(unsigned int size)
{
	mallocated -= size;
}


