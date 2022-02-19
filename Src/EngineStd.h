#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#define NOMINMAX
#include <Windows.h>

// C RunTime Header Files
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <sstream>
#include <strstream>
#include <stdint.h>

#include <assert.h>

// STL
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <set>

// Memory Leaks
#include <crtdbg.h>

// Include OGRE
#include "OGRE/OgreRoot.h"
#include "OGRE/OgreWindowEventUtilities.h"
#include "OGRE/OgreRenderWindow.h"
#include "OGRE/OgreManualObject.h"
#include "OGRE/OgreEntity.h"
#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#if defined(_DEBUG)
	#include <vld.h>
#endif

// audio
#include "ZFXAudio.h"

#if defined(_DEBUG)
#	define K_new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define K_new new
#endif





// alignment trick, only for classes/structs
template <typename T>
struct alignof
{
  enum { s = sizeof (T), value = s ^ (s & (s - 1)) };
};
#define ALIGNOF(type) alignof<type>::value

// BOOST
#include <boost\config.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\utility.hpp>
#include <boost\enable_shared_from_this.hpp>

using boost::shared_ptr;

// Input
struct AppMsg
{
	HWND mhWnd;
	UINT muMsg;
	WPARAM mwParam;
	LPARAM mlParam;
};

typedef uint64_t U64;


#define ERR(x) {MessageBox(NULL, x, "Error", MB_OK);}
// stands for all events
char const * const kpAllEventTypes = "*";

#include "Math/TSingleton.h"
#include "Math/Clock.h"
#include "Math/CRandom.h"
#include "Math/Handle.h"
#include "Interfaces.h"
#include "EventManager/StackAllocator.h"
#include "TextRenderer.h"

// MATH
#define MEGABYTE (1024 * 1024)
#define SIXTY_HERTZ (16.66f)

#define fOPAQUE					(1.0f)
#define iOPAQUE					(1)
#define fTRANSPARENT			(0.0f)
#define iTRANSPARENT			(0)
#define E_MIN(a, b)				((a) < (b) ? (a) : (b))						
#define E_MAX(a, b)				((a) > (b) ? (a) : (b))						
#define EPI						(3.1415926535897932384626433832795f)	
#define EPI2				    (1.5707963)
#define E2PI					(6.2831853)
#define E_DEG_TO_RAD(x)			((x) * 0.0174532925199432957692369076848f)	
#define E_RAD_TO_DEG(x)			((x) * 57.295779513082320876798154814105f)


#pragma warning( disable : 4996 )
#pragma warning(disable : 4995)

// SAFE_DELETE
#if !defined(SAFE_DELETE)
	#define SAFE_DELETE(x) if(x) delete x; x=NULL;
#endif

#if !defined(SAFE_DELETE_ARRAY)
	#define SAFE_DELETE_ARRAY(x) if (x) delete [] x; x=NULL; 
#endif

#if !defined(SAFE_RELEASE)
	#define SAFE_RELEASE(x) if(x) x->Release(); x=NULL;
#endif


#ifdef _DEBUG
	#pragma comment(lib, "Winmm.lib")
	#pragma comment(lib, "OgreMain_d.lib")
	#pragma comment(lib, "OgrePaging_d.lib")
	#pragma comment(lib, "OgreProperty_d.lib")
	#pragma comment(lib, "OgreRTShaderSystem_d.lib")
	#pragma comment(lib, "OgreTerrain_d.lib")
	#pragma comment(lib, "RenderSystem_Direct3D9_d.lib")
	#pragma comment(lib, "RenderSystem_GL_d.lib")
	#pragma comment(lib, "zlib.lib")
	#pragma comment(lib, "OIS_d.lib")
#else
	#pragma comment(lib, "OgreMain.lib")
	#pragma comment(lib, "OgrePaging.lib")
	#pragma comment(lib, "OgreProperty.lib")
	#pragma comment(lib, "OgreRTShaderSystem.lib")
	#pragma comment(lib, "OgreTerrain.lib")
	#pragma comment(lib, "RenderSystem_Direct3D9.lib")
	#pragma comment(lib, "RenderSystem_GL.lib")
	#pragma comment(lib, "zlib.lib")
	#pragma comment(lib, "OIS.lib")
#endif

extern int WINAPI Game(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPWSTR lpCmdLine,
				   int nCmdShow);