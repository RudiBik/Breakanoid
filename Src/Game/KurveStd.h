#pragma once

#if defined(_DEBUG)
	#include <vld.h>
#endif

#include "..\EngineStd.h"
#include "KurveActors.h"
#include "KurveEvents.h"
#include "KurveView.h"

#ifndef _DEBUG
#pragma comment(lib, "engine.lib")
#else
#pragma comment(lib, "dengine.lib")
#endif

extern char buffer[1024];