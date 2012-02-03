
#include "Mutex.h"

Mutex::Mutex()
{
	InitializeCriticalSection(&m_cs);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&m_cs);
}

void Mutex::acquire()
{
	EnterCriticalSection(&m_cs);
}

void Mutex::release()
{
	LeaveCriticalSection(&m_cs);
}

MutexScope::MutexScope(Mutex& mutex)
	: m_mutex(mutex)
{
	m_mutex.acquire();
}

MutexScope::~MutexScope()
{
	m_mutex.release();
}
