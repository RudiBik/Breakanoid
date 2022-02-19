#pragma once

template <class T>
class TSingleton {
protected:
	static T *mpSingleton;

public:
	virtual ~TSingleton() {}

	inline static T* Get() {
		if(!mpSingleton)
			mpSingleton = K_new T;

		return mpSingleton;
	}

	static void Del() {
		if(mpSingleton) {
			delete mpSingleton;
			mpSingleton = NULL;
		}
	}
};

template <class T>
T* TSingleton<T>::mpSingleton = NULL;