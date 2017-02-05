#pragma once

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

template<typename T> void SafeDelete(T*& a)
{
	delete a;
	a = NULL;
}
template<typename T> void SafeDeletes(T*& a)
{
	delete[] a;
	a = NULL;
}

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

template <typename T>
inline void MySwap(T &t1, T &t2)
{
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
};
