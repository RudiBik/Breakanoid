#include "..\EngineStd.h"

#include "ZipFile.h"

#include "zlib\zlib.h"

#include "ZipFile.h"
#include "zlib\zlib.h"
#include <string.h>

// --------------------------------------------------------------------------
// Basic types.
// --------------------------------------------------------------------------
typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

// --------------------------------------------------------------------------
// ZIP file structures. Note these have to be packed.
// --------------------------------------------------------------------------

#pragma pack(1)
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipLocalHeader
{
  enum
  {
    SIGNATURE = 0x04034b50,
  };
  dword   sig;
  word    version;
  word    flag;
  word    compression;      // Z_NO_COMPRESSION or Z_DEFLATED
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;
  dword   ucSize;
  word    fnameLen;         // Filename string follows header.
  word    xtraLen;          // Extra field follows filename.
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipDirHeader
{
  enum
  {
    SIGNATURE = 0x06054b50
  };
  dword   sig;
  word    nDisk;
  word    nStartDisk;
  word    nDirEntries;
  word    totalDirEntries;
  dword   dirSize;
  dword   dirOffset;
  word    cmntLen;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct ZipFile::TZipDirFileHeader
{
  enum
  {
    SIGNATURE   = 0x02014b50
  };
  dword   sig;
  word    verMade;
  word    verNeeded;
  word    flag;
  word    compression;      // COMP_xxxx
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;            // Compressed size
  dword   ucSize;           // Uncompressed size
  word    fnameLen;         // Filename string follows header.
  word    xtraLen;          // Extra field follows filename.
  word    cmntLen;          // Comment field follows extra field.
  word    diskStart;
  word    intAttr;
  dword   extAttr;
  dword   hdrOffset;

  char *GetName   () const { return (char *)(this + 1);   }
  char *GetExtra  () const { return GetName() + fnameLen; }
  char *GetComment() const { return GetExtra() + xtraLen; }
};

#pragma pack()

// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object and read the zip file directory.
// Parameters:    A stdio FILE* used for reading.
// --------------------------------------------------------------------------
bool ZipFile::Init(const wchar_t *resFileName)
{
  End();

  mpFile = _wfopen(resFileName, L"rb");
  if (!mpFile)
    return false;

  // Assuming no extra comment at the end, read the whole end record.
  TZipDirHeader dh;

  fseek(mpFile, -(int)sizeof(dh), SEEK_END);
  long dhOffset = ftell(mpFile);
  memset(&dh, 0, sizeof(dh));
  fread(&dh, sizeof(dh), 1, mpFile);

  // Check
  if (dh.sig != TZipDirHeader::SIGNATURE)
    return false;

  // Go to the beginning of the directory.
  fseek(mpFile, dhOffset - dh.dirSize, SEEK_SET);

  // Allocate the data buffer, and read the whole thing.
  mpDirData = K_new char[dh.dirSize + dh.nDirEntries*sizeof(*mpapDir)];
  if (!mpDirData)
    return false;
  memset(mpDirData, 0, dh.dirSize + dh.nDirEntries*sizeof(*mpapDir));
  fread(mpDirData, dh.dirSize, 1, mpFile);

  // Now process each entry.
  char *pfh = mpDirData;
  mpapDir = (const TZipDirFileHeader **)(mpDirData + dh.dirSize);

  bool success = true;

  for (int i = 0; i < dh.nDirEntries && success; i++)
  {
    TZipDirFileHeader &fh = *(TZipDirFileHeader*)pfh;

    // Store the address of nth file for quicker access.
    mpapDir[i] = &fh;

    // Check the directory entry integrity.
    if (fh.sig != TZipDirFileHeader::SIGNATURE)
      success = false;
    else
    {
      pfh += sizeof(fh);

      // Convert UNIX slashes to DOS backlashes.
      for (int j = 0; j < fh.fnameLen; j++)
        if (pfh[j] == '/')
          pfh[j] = '\\';

	  char fileName[_MAX_PATH];
	  memcpy(fileName, pfh, fh.fnameLen);
	  fileName[fh.fnameLen]=0;
	  _strlwr(fileName);
	  std::string spath = fileName;
	  mZipContentsMap[spath] = i;

      // Skip name, extra and comment fields.
      pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
    }
  }
  if (!success)
  {
    SAFE_DELETE_ARRAY(mpDirData);
  }
  else
  {
    mnEntries = dh.nDirEntries;
  }

  return success;
}

int ZipFile::Find(const char *path) const
{
	char lwrPath[_MAX_PATH];
	strcpy(lwrPath, path);
	_strlwr(lwrPath);
	ZipContentsMap::const_iterator i = mZipContentsMap.find(lwrPath);
	if (i==mZipContentsMap.end())
		return -1;

	return (*i).second;
}



// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Finish the object
// Parameters:    
// --------------------------------------------------------------------------
void ZipFile::End()
{
	mZipContentsMap.empty();
    SAFE_DELETE_ARRAY(mpDirData);
    mnEntries = 0;
}

// --------------------------------------------------------------------------
// Function:      GetFilename
// Purpose:       Return the name of a file
// Parameters:    The file index and the buffer where to store the filename
// --------------------------------------------------------------------------
void ZipFile::GetFilename(int i, char *pszDest)  const
{
  if (pszDest != NULL)
  {
    if (i < 0 || i >= mnEntries)
      *pszDest = '\0';
    else
    {
      memcpy(pszDest, mpapDir[i]->GetName(), mpapDir[i]->fnameLen);
      pszDest[mpapDir[i]->fnameLen] = '\0';
    }
  }
}


// --------------------------------------------------------------------------
// Function:      GetFileLen
// Purpose:       Return the length of a file so a buffer can be allocated
// Parameters:    The file index.
// --------------------------------------------------------------------------
int ZipFile::GetFileLen(int i) const
{
  if (i < 0 || i >= mnEntries)
    return -1;
  else
    return mpapDir[i]->ucSize;
}

// --------------------------------------------------------------------------
// Function:      ReadFile
// Purpose:       Uncompress a complete file
// Parameters:    The file index and the pre-allocated buffer
// --------------------------------------------------------------------------
bool ZipFile::ReadFile(int i, void *pBuf)
{
  if (pBuf == NULL || i < 0 || i >= mnEntries)
    return false;

  // Quick'n dirty read, the whole file at once.
  // Ungood if the ZIP has huge files inside

  // Go to the actual file and read the local header.
  fseek(mpFile, mpapDir[i]->hdrOffset, SEEK_SET);
  TZipLocalHeader h;

  memset(&h, 0, sizeof(h));
  fread(&h, sizeof(h), 1, mpFile);
  if (h.sig != TZipLocalHeader::SIGNATURE)
    return false;

  // Skip extra fields
  fseek(mpFile, h.fnameLen + h.xtraLen, SEEK_CUR);

  if (h.compression == Z_NO_COMPRESSION)
  {
    // Simply read in raw stored data.
    fread(pBuf, h.cSize, 1, mpFile);
    return true;
  }
  else if (h.compression != Z_DEFLATED)
    return false;

  // Alloc compressed data buffer and read the whole stream
  char *pcData = K_new char[h.cSize];
  if (!pcData)
    return false;

  memset(pcData, 0, h.cSize);
  fread(pcData, h.cSize, 1, mpFile);

  bool ret = true;

  // Setup the inflate stream.
  z_stream stream;
  int err;

  stream.next_in = (Bytef*)pcData;
  stream.avail_in = (uInt)h.cSize;
  stream.next_out = (Bytef*)pBuf;
  stream.avail_out = h.ucSize;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  // Perform inflation. wbits < 0 indicates no zlib header inside the data.
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err == Z_OK)
  {
    err = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (err == Z_STREAM_END)
      err = Z_OK;
    inflateEnd(&stream);
  }
  if (err != Z_OK)
    ret = false;

  delete[] pcData;
  return ret;
}



// --------------------------------------------------------------------------
// Function:      ReadLargeFile
// Purpose:       Uncompress a complete file with callbacks.
// Parameters:    The file index and the pre-allocated buffer
// --------------------------------------------------------------------------
bool ZipFile::ReadLargeFile(int i, void *pBuf, void (*callback)(int, bool &))

{
  if (pBuf == NULL || i < 0 || i >= mnEntries)
    return false;

  // Quick'n dirty read, the whole file at once.
  // Ungood if the ZIP has huge files inside

  // Go to the actual file and read the local header.
  fseek(mpFile, mpapDir[i]->hdrOffset, SEEK_SET);
  TZipLocalHeader h;

  memset(&h, 0, sizeof(h));
  fread(&h, sizeof(h), 1, mpFile);
  if (h.sig != TZipLocalHeader::SIGNATURE)
    return false;

  // Skip extra fields
  fseek(mpFile, h.fnameLen + h.xtraLen, SEEK_CUR);

  if (h.compression == Z_NO_COMPRESSION)
  {
    // Simply read in raw stored data.
    fread(pBuf, h.cSize, 1, mpFile);
    return true;
  }
  else if (h.compression != Z_DEFLATED)
    return false;

  // Alloc compressed data buffer and read the whole stream
  char *pcData = K_new char[h.cSize];
  if (!pcData)
    return false;

  memset(pcData, 0, h.cSize);
  fread(pcData, h.cSize, 1, mpFile);

  bool ret = true;

  // Setup the inflate stream.
  z_stream stream;
  int err;

  stream.next_in = (Bytef*)pcData;
  stream.avail_in = (uInt)h.cSize;
  stream.next_out = (Bytef*)pBuf;
  stream.avail_out = (128 * 1024); //  read 128k at a time h.ucSize;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  // Perform inflation. wbits < 0 indicates no zlib header inside the data.
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err == Z_OK)
  {
	  uInt count = 0;
	  bool cancel = false;
		while (stream.total_in < (uInt)h.cSize && !cancel)
		{
			err = inflate(&stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END)
			{
				err = Z_OK;
				break;
			}
			else if (err != Z_OK)
			{
				assert(0 && "Something happened.");
				break;
			}

			stream.avail_out = (128 * 1024); 
			stream.next_out += stream.total_out;

			callback(count * 100 / h.cSize, cancel);
		}
		inflateEnd(&stream);
  }
  if (err != Z_OK)
    ret = false;

  delete[] pcData;
  return ret;
}

