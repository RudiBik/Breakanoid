#pragma once

typedef std::map<std::string, int> ZipContentsMap;

class ZipFile
{
  public:
    ZipFile() { mnEntries=0; mpFile=NULL; mpDirData=NULL; }
    virtual ~ZipFile() { End(); fclose(mpFile); }

    bool Init(const wchar_t *resFileName);
    void End();

    int GetNumFiles()const { return mnEntries; }
    void GetFilename(int i, char *pszDest) const;
    int GetFileLen(int i) const;
    bool ReadFile(int i, void *pBuf);

	// Added to show multi-threaded decompression
	bool ReadLargeFile(int i, void *pBuf, void (*callback)(int, bool &));

	int Find(const char *path) const;

	ZipContentsMap mZipContentsMap;

  private:
    struct TZipDirHeader;
    struct TZipDirFileHeader;
    struct TZipLocalHeader;

    FILE *mpFile;		// Zip file
    char *mpDirData;	// Raw data buffer.
    int  mnEntries;	// Number of entries.

    // Pointers to the dir entries in pDirData.
    const TZipDirFileHeader **mpapDir;   
};
