
#ifndef Mutex_h
#define Mutex_h

#include <Windows.h>

class Mutex
{
public:
	Mutex();
	~Mutex();

	void acquire();
	void release();

private:
	CRITICAL_SECTION m_cs;
};

class MutexScope
{
public:
	MutexScope(Mutex& mutex);
	~MutexScope();

private:
	Mutex& m_mutex;
};

#endif
