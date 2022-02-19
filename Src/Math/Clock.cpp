#include "..\EngineStd.h"

double g_fElapsedTime = 1/30.0f;
float g_fRealTime = 1/30.0f;
float g_fMilliseconds = 0.0f;

Clock *g_pClock = NULL;
LARGE_INTEGER Clock::sCyclesPerSecond;
