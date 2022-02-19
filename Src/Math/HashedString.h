#pragma once


// Removes characters up to the first '\n'
extern void RemoveFirstLine(std::wstring &src, std::wstring &result);

// Removes leading white space
extern void TrimLeft(std::wstring &s);

// Counts the number of lines in a block of text
extern int CountLines(const std::wstring &s);


extern HRESULT AnsiToWideCch( WCHAR* dest, const CHAR* src, int charCount);  
extern HRESULT WideToAnsiCch( CHAR* dest, const WCHAR* src, int charCount);  
extern HRESULT GenericToAnsiCch( CHAR* dest, const TCHAR* src, int charCount); 
extern HRESULT GenericToWideCch( WCHAR* dest, const TCHAR* src, int charCount); 
extern HRESULT AnsiToGenericCch( TCHAR* dest, const CHAR* src, int charCount); 
extern HRESULT WideToGenericCch( TCHAR* dest, const WCHAR* src, int charCount);

#pragma warning(push)
#pragma warning(disable : 4311)

// A hashed string.  It retains the initial (ANSI) string in addition to the hash value for easy reference.

// class HashedString				- Chapter 10, page 274

class HashedString
{
public:
	explicit HashedString( char const * const pIdentString ) : midentStr( pIdentString )
	{
		if(strcmp(pIdentString, kpAllEventTypes) == 0) {
			mident = 0;
		} else {
			mident = hash_name(pIdentString);
		}
	}

	unsigned long getHashValue( void ) const
	{

		return reinterpret_cast<unsigned long>( mident );
	}

	const std::string & getStr() const
	{
		return midentStr;
	}

	static
	void * hash_name( char const *  pIdentStr );

	bool operator< ( HashedString const & o ) const
	{
		bool r = ( getHashValue() < o.getHashValue() );
		return r;
	}

	bool operator== ( HashedString const & o ) const
	{
		bool r = ( getHashValue() == o.getHashValue() );
		return r;
	}

private:

	// note: mident is stored as a void* not an int, so that in
	// the debugger it will show up as hex-values instead of
	// integer values. This is a bit more representative of what
	// we're doing here and makes it easy to allow external code
	// to assign event types as desired.

	void *             mident;
	std::string		   midentStr;
};
//Remove the warning for warning #4311...
#pragma warning(pop)
