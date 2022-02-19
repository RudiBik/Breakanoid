#include "..\EngineStd.h"

#include "HashedString.h"

void RemoveFirstLine(std::wstring &src, std::wstring &result)
{
	int breakPosition = (int)src.find('\n');
	result = L"";
	if(breakPosition != std::string::npos)	//if found...
	{
		int len = (int)src.length();
		result = src.substr(0, breakPosition);
		src = src.substr(breakPosition+1, (len-breakPosition)-1);		// skip the '/n'
	}
	else
	{
		result = src;
		src = L"";
	}
}


// Remove all leading whitespace
void TrimLeft(std::wstring &s)
{
	// find first non-space character
	int i = 0;
	int len = (int)s.length();
	while( i <  len )
	{
		TCHAR ch = s[i];			
		int white = 
		#ifdef UNICODE
				iswspace( ch );
		#else
				isspace( ch );
		#endif
		if (!white)
			break;
		++i;
	}

	if (i<len)
		s = s.substr(i);
}


int CountLines(const std::wstring &s)
{
	int lines = 0;
	int breakPos = 0;

	do 
	{
		++lines;
		breakPos = (int)s.find('\n', breakPos+1);
	} while (breakPos != std::string::npos);

	return lines;
}	






//-----------------------------------------------------------------------------
// Name: AnsiToWideCch()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT AnsiToWideCch( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                                       wstrDestination, cchDestChar );
    wstrDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: WideToAnsi()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT WideToAnsiCch( CHAR* strDestination, const WCHAR* wstrSource, 
                                     int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                                       cchDestChar*sizeof(CHAR), NULL, NULL );
    strDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: GenericToAnsi()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT GenericToAnsiCch( CHAR* strDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( strDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    return WideToAnsiCch( strDestination, tstrSource, cchDestChar );
#else
    strncpy( strDestination, tstrSource, cchDestChar );
    strDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif   
}




//-----------------------------------------------------------------------------
// Name: GenericToWide()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT GenericToWideCch( WCHAR* wstrDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( wstrDestination, tstrSource, cchDestChar );
    wstrDestination[cchDestChar-1] = L'\0';
    return S_OK;
#else
    return AnsiToWideCch( wstrDestination, tstrSource, cchDestChar );
#endif    
}



//-----------------------------------------------------------------------------
// Name: AnsiToGeneric()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT AnsiToGenericCch( TCHAR* tstrDestination, const CHAR* strSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;
        
#ifdef _UNICODE
    return AnsiToWideCch( tstrDestination, strSource, cchDestChar );
#else
    strncpy( tstrDestination, strSource, cchDestChar );
    tstrDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif    
}





//-----------------------------------------------------------------------------
// Name: AnsiToGeneric()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT WideToGenericCch( TCHAR* tstrDestination, const WCHAR* wstrSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( tstrDestination, wstrSource, cchDestChar );
    tstrDestination[cchDestChar-1] = L'\0';    
    return S_OK;
#else
    return WideToAnsiCch( tstrDestination, wstrSource, cchDestChar );
#endif
}

void *
HashedString::hash_name( char const * pIdentStr )
{
	// Relatively simple hash of arbitrary text string into a
	// 32-bit identifier Output value is
	// input-valid-deterministic, but no guarantees are made
	// about the uniqueness of the output per-input
	//
	// Input value is treated as lower-case to cut down on false
	// separations cause by human mistypes. Sure, it could be
	// construed as a programming error to mix up your cases, and
	// it cuts down on permutations, but in Real World Usage
	// making this text case-sensitive will likely just lead to
	// Pain and Suffering.
	//
	// This code lossely based upon the adler32 checksum by Mark
	// Adler and published as part of the zlib compression
	// library sources.

	// largest prime smaller than 65536
	unsigned long BASE = 65521L;

	// NMAX is the largest n such that 255n(n+1)/2 +
	// (n+1)(BASE-1) <= 2^32-1
	unsigned long NMAX = 5552;

#define DO1(buf,i)  {s1 += tolower(buf[i]); s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

    if (pIdentStr == NULL)
		return NULL;

    unsigned long s1 = 0;
    unsigned long s2 = 0;

	for ( size_t len = strlen( pIdentStr ); len > 0 ; )
	{
        unsigned long k = len < NMAX ? len : NMAX;

		len -= k;

		while (k >= 16)
		{
            DO16(pIdentStr);
			pIdentStr += 16;
            k -= 16;
        }
		
        if (k != 0) do
		{
			s1 += tolower( *pIdentStr++ );
			s2 += s1;
        } while (--k);
		
        s1 %= BASE;
        s2 %= BASE;
    }

#pragma warning(push)
#pragma warning(disable : 4312)

    return reinterpret_cast<void *>( (s2 << 16) | s1 );

#pragma warning(pop)
#undef DO1
#undef DO2
#undef DO4
#undef DO8
#undef DO16
}
