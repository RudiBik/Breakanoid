#pragma once

extern double g_fElapsedTime;
extern float g_fRealTime;
extern float g_fMilliseconds;


class Clock {
public:
	U64 mtimeCycles;
	U64 mlastCycles;
	U64 mrealLast;

	float		  mfTimeScale;
	float		  mfFPS;
	bool		  mbPaused;

	static LARGE_INTEGER sCyclesPerSecond;

	static inline U64 secondsToCycles(float fTime) {
		return (U64)(fTime * sCyclesPerSecond.QuadPart);
	}

	static inline double cyclesToSeconds(U64 cycles) {
		return (double)cycles / sCyclesPerSecond.QuadPart;
	}

	static bool Init() {
		if(!QueryPerformanceFrequency(&sCyclesPerSecond)) {
			ERR("QueryPerformanceFrequency not allowed.");
			return false;
		} 

		return true;
	}
	
	Clock(float fStartTime = 0.0f) :
		mtimeCycles(secondsToCycles(fStartTime)),
		mlastCycles(secondsToCycles(fStartTime)),
		mbPaused(false),
		mfTimeScale(1.0f),
		mfFPS(0.0f)
		{}

	LONGLONG GetTimeCycles() { return mtimeCycles; }
	float GetFPS() { return mfFPS; }

	double CalcDeltaSeconds(Clock other) {
		LONGLONG dt = mtimeCycles - other.mtimeCycles;
		return cyclesToSeconds(dt);
	}

	void Update() {
		LARGE_INTEGER cycles;
		QueryPerformanceCounter(&cycles);
		U64 delta = cycles.QuadPart - mrealLast;

		if(!mbPaused) 
			mtimeCycles += U64((float)delta * mfTimeScale);

		g_fRealTime = (float)cyclesToSeconds(delta);
		g_fElapsedTime = cyclesToSeconds(mtimeCycles - mlastCycles);
		g_fMilliseconds = (float)g_fElapsedTime * 1000.0f;

		mrealLast = cycles.QuadPart;
		mlastCycles = mtimeCycles;
	}

	void SetPaused(bool paused) { mbPaused = paused; }
	bool IsPaused() { return mbPaused; }

	void SetTimeScale(float scale) { mfTimeScale = scale; }
	float GetTimeScale() { return mfTimeScale; }

	void SingleStep() {
		if(mbPaused) {
			U64 dtScaledCycles = secondsToCycles((1.0f/30.0f) * mfTimeScale);
			mtimeCycles += dtScaledCycles;
		}
	}
};

extern Clock *g_pClock;